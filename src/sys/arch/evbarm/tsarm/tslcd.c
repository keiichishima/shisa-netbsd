/* $NetBSD: tslcd.c,v 1.4 2005/02/04 06:02:36 joff Exp $ */

/*-
 * Copyright (c) 1998 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jesse Off.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the NetBSD
 *	Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: tslcd.c,v 1.4 2005/02/04 06:02:36 joff Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/poll.h>
#include <sys/conf.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/device.h>
#include <sys/callout.h>
#include <sys/select.h>
#include <sys/conf.h>

#include <machine/bus.h>
#include <machine/autoconf.h>

#include <dev/wscons/wsdisplayvar.h>
#include <dev/wscons/wsconsio.h>
#include <dev/wscons/wscons_callbacks.h>

#include <arm/ep93xx/ep93xxreg.h>
#include <dev/ic/hd44780reg.h>
#include <dev/ic/hd44780var.h>
#include <evbarm/tsarm/tspldvar.h>
#include <evbarm/tsarm/tsarmreg.h>

struct tslcd_softc {
	struct device sc_dev;
	struct hd44780_chip sc_hlcd;
	bus_space_tag_t sc_iot;
	bus_space_handle_t sc_gpioh;
};

static int	tslcd_match(struct device *, struct cfdata *, void *);
static void	tslcd_attach(struct device *, struct device *, void *);

static void	tslcd_writereg(struct hd44780_chip *, u_int32_t, u_int8_t);
static u_int8_t	tslcd_readreg(struct hd44780_chip *, u_int32_t);

dev_type_open(tslcdopen);
dev_type_close(tslcdclose);
dev_type_read(tslcdread);
dev_type_write(tslcdwrite);
dev_type_ioctl(tslcdioctl);
dev_type_poll(tslcdpoll);

const struct cdevsw tslcd_cdevsw = {
	tslcdopen, tslcdclose, tslcdread, tslcdwrite, tslcdioctl,
	nostop, notty, tslcdpoll, nommap,
};

extern const struct wsdisplay_emulops hlcd_emulops;
extern const struct wsdisplay_accessops hlcd_accessops;
extern struct cfdriver tslcd_cd;

CFATTACH_DECL(tslcd, sizeof(struct tslcd_softc),
    tslcd_match, tslcd_attach, NULL, NULL);

static const struct wsscreen_descr tslcd_stdscreen = {
	"std_tslcd", 24, 2,
	&hlcd_emulops,
	5, 7,
	0,
};

static const struct wsscreen_descr *_tslcd_scrlist[] = {
	&tslcd_stdscreen,
};

static const struct wsscreen_list tslcd_screenlist = {
	sizeof(_tslcd_scrlist) / sizeof(struct wsscreen_descr *),
	_tslcd_scrlist,
};

static int
tslcd_match(parent, match, aux)
	struct device *parent;
	struct cfdata *match;
	void *aux;
{
	return 1;
}

#define GPIO_GET(x)	bus_space_read_1(sc->sc_iot, sc->sc_gpioh, \
	(EP93XX_GPIO_ ## x))

#define GPIO_SET(x, y)	bus_space_write_1(sc->sc_iot, sc->sc_gpioh, \
	(EP93XX_GPIO_ ## x), (y))

#define GPIO_SETBITS(x, y)	bus_space_write_1(sc->sc_iot, sc->sc_gpioh, \
	(EP93XX_GPIO_ ## x), GPIO_GET(x) | (y))

#define GPIO_CLEARBITS(x, y)	bus_space_write_1(sc->sc_iot, sc->sc_gpioh, \
	(EP93XX_GPIO_ ## x), GPIO_GET(x) & (~(y)))

static void
tslcd_attach(parent, self, aux)
	struct device *parent;
	struct device *self;
	void *aux;
{
	struct tslcd_softc *sc = (void *)self;
	struct tspld_attach_args *taa = aux;
	struct wsemuldisplaydev_attach_args waa;

	sc->sc_iot = taa->ta_iot;
	if (bus_space_map(sc->sc_iot, EP93XX_APB_HWBASE + EP93XX_APB_GPIO,
		EP93XX_APB_GPIO_SIZE, 0, &sc->sc_gpioh))
		panic("tslcd_attach: couldn't map GPIO registers");

	sc->sc_hlcd.sc_dev_ok = 1;
	sc->sc_hlcd.sc_cols = 24;
	sc->sc_hlcd.sc_vcols = 40;
	sc->sc_hlcd.sc_flags = HD_8BIT | HD_MULTILINE;
	sc->sc_hlcd.sc_dev = self;

	sc->sc_hlcd.sc_writereg = tslcd_writereg;
	sc->sc_hlcd.sc_readreg = tslcd_readreg;
	
	GPIO_SET(PADDR, 0);		/* Port A to inputs */
	GPIO_SETBITS(PHDDR, 0x38);	/* Bits 3:5 of Port H to outputs */
	GPIO_CLEARBITS(PHDR, 0x18);	/* De-assert EN, De-assert RS */

	printf("\n");

	hd44780_attach_subr(&sc->sc_hlcd);

	waa.console = 0;
	waa.scrdata = &tslcd_screenlist;
	waa.accessops = &hlcd_accessops;
	waa.accesscookie = &sc->sc_hlcd.sc_screen;
	config_found(self, &waa, wsemuldisplaydevprint);
}

static void
tslcd_writereg(hd, rs, cmd)
	struct hd44780_chip *hd;
	u_int32_t rs;
	u_int8_t cmd;
{
	struct tslcd_softc *sc = (struct tslcd_softc *)hd->sc_dev;
	u_int8_t ctrl;

	if (hd->sc_dev_ok == 0)
		return;

	/* Step 1: Apply RS & WR, Send data */
	ctrl = GPIO_GET(PHDR);
	GPIO_SET(PADDR, 0xff); /* set port A to outputs */
	GPIO_SET(PADR, cmd);
	if (rs) {
		ctrl |= 0x10;	/* assert RS */
		ctrl &= ~0x20;	/* assert WR */
	} else {
		ctrl &= ~0x30;	/* de-assert WR, de-assert RS */
	}
	GPIO_SET(PHDR, ctrl);

	/* Step 2: setup time delay */
	delay(1);

	/* Step 3: assert EN */
	ctrl |= 0x8;
	GPIO_SET(PHDR, ctrl);

	/* Step 4: pulse time delay */
	delay(1);

	/* Step 5: de-assert EN */
	ctrl &= ~0x8;
	GPIO_SET(PHDR, ctrl);

	/* Step 6: hold time delay */
	delay(1);
	
	/* Step 7: de-assert WR */
	ctrl |= 0x2;
	GPIO_SET(PHDR, ctrl); 

	/* Step 8: minimum delay till next bus-cycle */
	delay(1000);
}

static u_int8_t
tslcd_readreg(hd, rs)
	struct hd44780_chip *hd;
	u_int32_t rs;
{
	struct tslcd_softc *sc = (struct tslcd_softc *)hd->sc_dev;
	u_int8_t ret, ctrl;

	if (hd->sc_dev_ok == 0)
		return 0;

	/* Step 1: Apply RS & WR, Send data */
	ctrl = GPIO_GET(PHDR);
	GPIO_SET(PADDR, 0x0);	/* set port A to inputs */
	if (rs) {
		ctrl |= 0x30;	/* de-assert WR, assert RS */
	} else {
		ctrl |= 0x20;	/* de-assert WR */
		ctrl &= ~0x10;	/* de-assert RS */
	}
	GPIO_SET(PHDR, ctrl);

	/* Step 2: setup time delay */
	delay(1);

	/* Step 3: assert EN */
	ctrl |= 0x8;
	GPIO_SET(PHDR, ctrl);

	/* Step 4: pulse time delay */
	delay(1);

	/* Step 5: de-assert EN */
	ret = GPIO_GET(PADR) & 0xff;
	ctrl &= ~0x8;
	GPIO_SET(PHDR, ctrl);

	/* Step 6: hold time delay + min bus cycle interval*/
	delay(1000);
	return ret;
}

int
tslcdopen(dev, flag, mode, p)
	dev_t dev;
	int flag, mode;
	struct proc *p;
{
	struct tslcd_softc *sc = device_lookup(&tslcd_cd, minor(dev));

	if (sc->sc_hlcd.sc_dev_ok == 0)
		return hd44780_init(&sc->sc_hlcd);
	else
		return 0;
}

int
tslcdclose(dev, flag, mode, p)
	dev_t dev;
	int flag, mode;
	struct proc *p;
{
	return 0;
}

int
tslcdread(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
	int flag;
{
	return EIO;
}

int
tslcdwrite(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
	int flag;
{
	int error;
	struct hd44780_io io;
	struct tslcd_softc *sc = device_lookup(&tslcd_cd, minor(dev));

	if (sc->sc_hlcd.sc_dev_ok == 0)
		return EIO;

	io.dat = 0;
	io.len = uio->uio_resid;
	if (io.len > HD_MAX_CHARS)
		io.len = HD_MAX_CHARS;

	if ((error = uiomove((void*)io.buf, io.len, uio)) != 0)
		return error;

	hd44780_ddram_redraw(&sc->sc_hlcd, &io);
	return 0;
}

int
tslcdioctl(dev, cmd, data, flag, p)
	dev_t dev;
	u_long cmd;
	caddr_t data;
	int flag;
	struct proc *p;
{
	struct tslcd_softc *sc = device_lookup(&tslcd_cd, minor(dev));
	return hd44780_ioctl_subr(&sc->sc_hlcd, cmd, data);
}

int
tslcdpoll(dev, events, p)
	dev_t dev;
	int events;
	struct proc *p;
{
	return 0;
}
