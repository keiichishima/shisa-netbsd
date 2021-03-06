/*	$NetBSD: sb_isa.c,v 1.31 2005/02/27 00:27:17 perry Exp $	*/

/*
 * Copyright (c) 1991-1993 Regents of the University of California.
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
 *	This product includes software developed by the Computer Systems
 *	Engineering Group at Lawrence Berkeley Laboratory.
 * 4. Neither the name of the University nor of the Laboratory may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: sb_isa.c,v 1.31 2005/02/27 00:27:17 perry Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/syslog.h>
#include <sys/device.h>
#include <sys/proc.h>

#include <machine/bus.h>

#include <sys/audioio.h>
#include <dev/audio_if.h>
#include <dev/midi_if.h>
#include <dev/mulaw.h>

#include <dev/isa/isavar.h>
#include <dev/isa/isadmavar.h>

#include <dev/isa/sbreg.h>
#include <dev/isa/sbvar.h>

#include <dev/isa/sbdspvar.h>

static	int sbfind(struct device *, struct sbdsp_softc *, int,
	    struct isa_attach_args *);

int	sb_isa_match(struct device *, struct cfdata *, void *);
void	sb_isa_attach(struct device *, struct device *, void *);

CFATTACH_DECL(sb_isa, sizeof(struct sbdsp_softc),
    sb_isa_match, sb_isa_attach, NULL, NULL);

/*
 * Probe / attach routines.
 */

/*
 * Probe for the soundblaster hardware.
 */
int
sb_isa_match(parent, match, aux)
	struct device *parent;
	struct cfdata *match;
	void *aux;
{
	struct isa_attach_args *ia = aux;
	struct sbdsp_softc probesc, *sc = &probesc;

	if (ia->ia_nio < 1)
		return (0);
	if (ia->ia_nirq < 1)
		return (0);
	if (ia->ia_ndrq < 1)
		return (0);

	if (ISA_DIRECT_CONFIG(ia))
		return (0);

	memset(sc, 0, sizeof *sc);
	strcpy(sc->sc_dev.dv_xname, "sb");
	sc->sc_dev.dv_cfdata = match;
	return sbfind(parent, sc, 1, aux);
}

static int
sbfind(parent, sc, probing, ia)
	struct device *parent;
	struct sbdsp_softc *sc;
	int probing;
	struct isa_attach_args *ia;
{
	int rc = 0;

	if (!SB_BASE_VALID(ia->ia_io[0].ir_addr)) {
		printf("sb: configured iobase 0x%x invalid\n",
		    ia->ia_io[0].ir_addr);
		return 0;
	}

	sc->sc_ic = ia->ia_ic;

	sc->sc_iot = ia->ia_iot;
	/* Map i/o space [we map 24 ports which is the max of the sb and pro */
	if (bus_space_map(sc->sc_iot, ia->ia_io[0].ir_addr, SBP_NPORT, 0,
	    &sc->sc_ioh))
		return 0;

	/* XXX These are only for setting chip configuration registers. */
	sc->sc_iobase = ia->ia_io[0].ir_addr;
	sc->sc_irq = ia->ia_irq[0].ir_irq;

	sc->sc_drq8 = ia->ia_drq[0].ir_drq;
	sc->sc_drq16 = ia->ia_drq[1].ir_drq;

	if (!sbmatch(sc))
		goto bad;

	rc = 1;

	if (probing) {
		ia->ia_nio = 1;
		if (ISSBPROCLASS(sc))
			ia->ia_io[0].ir_size = SBP_NPORT;
		else
			ia->ia_io[0].ir_size = SB_NPORT;

		if (!ISSB16CLASS(sc) && sc->sc_model != SB_JAZZ)
			ia->ia_ndrq = 1;
		else
			ia->ia_ndrq = 2;

		ia->ia_nirq = 1;
		ia->ia_irq[0].ir_irq = sc->sc_irq;

		ia->ia_niomem = 0;
	}

bad:
	bus_space_unmap(sc->sc_iot, sc->sc_ioh, SBP_NPORT);
	return rc;
}


/*
 * Attach hardware to driver, attach hardware driver to audio
 * pseudo-device driver .
 */
void
sb_isa_attach(parent, self, aux)
	struct device *parent, *self;
	void *aux;
{
	struct sbdsp_softc *sc = (struct sbdsp_softc *)self;
	struct isa_attach_args *ia = aux;

	if (!sbfind(parent, sc, 0, ia) ||
	    bus_space_map(sc->sc_iot, ia->ia_io[0].ir_addr,
	    ia->ia_io[0].ir_size, 0, &sc->sc_ioh)) {
		printf("%s: sbfind failed\n", sc->sc_dev.dv_xname);
		return;
	}

	sc->sc_ih = isa_intr_establish(ia->ia_ic, ia->ia_irq[0].ir_irq,
	    IST_EDGE, IPL_AUDIO, sbdsp_intr, sc);

	sbattach(sc);
}
