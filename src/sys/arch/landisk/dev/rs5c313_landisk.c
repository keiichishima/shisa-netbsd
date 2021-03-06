/*	$NetBSD: rs5c313_landisk.c,v 1.1 2006/09/07 01:55:03 uwe Exp $	*/

/*-
 * Copyright (c) 2002 The NetBSD Foundation, Inc.
 * All rights reserved.
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
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
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
__KERNEL_RCSID(0, "$NetBSD: rs5c313_landisk.c,v 1.1 2006/09/07 01:55:03 uwe Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/kernel.h>

#include <dev/clock_subr.h>
#include <dev/ic/rs5c313var.h>

#include <sh3/devreg.h>
#include <sh3/scireg.h>

#include <landisk/landisk/landiskreg.h>


/* autoconf glue */
static int rs5c313_landisk_match(struct device *, struct cfdata *, void *);
static void rs5c313_landisk_attach(struct device *, struct device *, void *);

CFATTACH_DECL(rs5c313_landisk, sizeof(struct rs5c313_softc),
    rs5c313_landisk_match, rs5c313_landisk_attach, NULL, NULL);


/* chip access methods */
static void rtc_begin(struct rs5c313_softc *);
static void rtc_ce(struct rs5c313_softc *, int);
static void rtc_dir(struct rs5c313_softc *, int);
static void rtc_clk(struct rs5c313_softc *, int);
static int  rtc_read(struct rs5c313_softc *);
static void rtc_write(struct rs5c313_softc *, int);

struct rs5c313_ops rs5c313_landisk_ops = {
	.rs5c313_op_begin = rtc_begin,
	.rs5c313_op_ce    = rtc_ce,
	.rs5c313_op_clk   = rtc_clk,
	.rs5c313_op_dir   = rtc_dir,
	.rs5c313_op_read  = rtc_read,
	.rs5c313_op_write = rtc_write,
};

#define ndelay(x) delay(x)



static int
rs5c313_landisk_match(struct device *parent, struct cfdata *cf, void *aux)
{
	static int matched = 0;

	if (matched)
		return 0;

	matched = 1;
	return 1;
}


static void
rs5c313_landisk_attach(struct device *parent, struct device *self, void *aux)
{
	struct rs5c313_softc *sc = (void *)self;

	sc->sc_ops = &rs5c313_landisk_ops;
	rs5c313_attach(sc);
}


static void
rtc_begin(struct rs5c313_softc *sc)
{

	SHREG_SCSPTR = SCSPTR_SPB1IO | SCSPTR_SPB1DT
		     | SCSPTR_SPB0IO | SCSPTR_SPB0DT;
	ndelay(100);
}


/*
 * CE pin
 */
static void
rtc_ce(struct rs5c313_softc *sc, int onoff)
{

	if (onoff)
		_reg_write_1(LANDISK_PWRMNG, PWRMNG_RTC_CE);
	else
		_reg_write_1(LANDISK_PWRMNG, 0);
	ndelay(600);
}


/*
 * SCLK pin is connnected to SPB0DT.
 * SPB0DT is always in output mode, we set SPB0IO in rtc_begin.
 */
static void
rtc_clk(struct rs5c313_softc *sc, int onoff)
{
	uint8_t r = SHREG_SCSPTR;

	if (onoff)
		r |= SCSPTR_SPB0DT;
	else
		r &= ~SCSPTR_SPB0DT;
	SHREG_SCSPTR = r;
}


/*
 * SIO pin is connected to SPB1DT.
 * SPB1DT is output when SPB1IO is set.
 */
static void
rtc_dir(struct rs5c313_softc *sc, int output)
{
	uint8_t r = SHREG_SCSPTR;

	if (output)
		r |= SCSPTR_SPB1IO;
	else
		r &= ~SCSPTR_SPB1IO;
	SHREG_SCSPTR = r;
}


/* 
 * Read bit from SPB1DT pin.
 */
static int
rtc_read(struct rs5c313_softc *sc)
{
	int bit;

	ndelay(300);

	bit = (SHREG_SCSPTR & SCSPTR_SPB1DT) ? 1 : 0;

	rtc_clk(sc, 0);
	ndelay(300);
	rtc_clk(sc, 1);

	return bit;
}


/* 
 * Write bit via SPB1DT pin.
 */
static void
rtc_write(struct rs5c313_softc *sc, int bit)
{
	uint8_t r = SHREG_SCSPTR;

	if (bit)
		r |= SCSPTR_SPB1DT;
	else
		r &= ~SCSPTR_SPB1DT;
	SHREG_SCSPTR = r;

	ndelay(300);

	rtc_clk(sc, 0);
	ndelay(300);
	rtc_clk(sc, 1);
}
