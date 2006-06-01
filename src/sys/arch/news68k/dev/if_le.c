/*	$NetBSD: if_le.c,v 1.12 2005/02/27 09:27:31 tsutsui Exp $	*/

/*-
 * Copyright (c) 1996 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Adam Glass and Gordon W. Ross.
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

/*
 * news68k/dev/if_le.c - based on newsmips/dev/if_le.c
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: if_le.c,v 1.12 2005/02/27 09:27:31 tsutsui Exp $");

#include "opt_inet.h"
#include "bpfilter.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/device.h>

#include <net/if.h>
#include <net/if_ether.h>
#include <net/if_media.h>

#ifdef INET
#include <netinet/in.h>
#include <netinet/if_inarp.h>
#endif

#include <machine/cpu.h>

#include <news68k/dev/hbvar.h>

#include <dev/ic/lancereg.h>
#include <dev/ic/lancevar.h>
#include <dev/ic/am7990reg.h>
#include <dev/ic/am7990var.h>

#include "ioconf.h"

/*
 * LANCE registers.
 * The real stuff is in dev/ic/am7990reg.h
 */
struct lereg1 {
	volatile uint16_t	ler1_rdp;	/* data port */
	volatile uint16_t	ler1_rap;	/* register select port */
};

/*
 * Ethernet software status per interface.
 * The real stuff is in dev/ic/am7990var.h
 */
struct	le_softc {
	struct	am7990_softc sc_am7990;	/* glue to MI code */

	struct	lereg1 *sc_r1;		/* LANCE registers */
};

static int  le_match(struct device *, struct cfdata *, void *);
static void le_attach(struct device *, struct device *, void *);

CFATTACH_DECL(le, sizeof(struct le_softc),
    le_match, le_attach, NULL, NULL);

extern volatile u_char *idrom_addr;
extern uint32_t lance_mem_phys;

#if defined(_KERNEL_OPT)
#include "opt_ddb.h"
#endif

#ifdef DDB
#define	integrate
#define hide
#else
#define	integrate	static __inline
#define hide		static
#endif

hide void lewrcsr(struct lance_softc *, uint16_t, uint16_t);
hide uint16_t lerdcsr(struct lance_softc *, uint16_t);
int leintr(int);

hide void
lewrcsr(struct lance_softc *sc, uint16_t port, uint16_t val)
{
	struct lereg1 *ler1 = ((struct le_softc *)sc)->sc_r1;

	ler1->ler1_rap = port;
	ler1->ler1_rdp = val;
}

hide uint16_t
lerdcsr(struct lance_softc *sc, uint16_t port)
{
	struct lereg1 *ler1 = ((struct le_softc *)sc)->sc_r1;
	uint16_t val;

	ler1->ler1_rap = port;
	val = ler1->ler1_rdp;
	return val;
}

int
le_match(struct device *parent, struct cfdata *cf, void *aux)
{
	struct hb_attach_args *ha = aux;
	int addr;

	if (strcmp(ha->ha_name, "le"))
		return 0;

	addr = IIOV(ha->ha_address);

	if (badaddr((void *)addr, 1))
		return 0;

	return 1;
}

void
le_attach(struct device *parent, struct device *self, void *aux)
{
	struct le_softc *lesc = (struct le_softc *)self;
	struct lance_softc *sc = &lesc->sc_am7990.lsc;
	struct hb_attach_args *ha = aux;
	u_char *p;

	lesc->sc_r1 = (void *)IIOV(ha->ha_address);

	if (ISIIOPA(ha->ha_address)) {
		sc->sc_mem = (u_char *)IIOV(lance_mem_phys);
		p = (u_char *)(idrom_addr + 0x10);
	} else {
		sc->sc_mem = lesc->sc_r1 - 0x10000;
		p = (u_char *)(lesc->sc_r1 + 0x8010);
	}

	sc->sc_memsize = 0x4000;	/* 16K */
	sc->sc_addr = lance_mem_phys & 0x00ffffff;
	sc->sc_conf3 = LE_C3_BSWP|LE_C3_BCON;

	sc->sc_enaddr[0] = (*p++ << 4);
	sc->sc_enaddr[0] |= *p++ & 0x0f;
	sc->sc_enaddr[1] = (*p++ << 4);
	sc->sc_enaddr[1] |= *p++ & 0x0f;
	sc->sc_enaddr[2] = (*p++ << 4);
	sc->sc_enaddr[2] |= *p++ & 0x0f;
	sc->sc_enaddr[3] = (*p++ << 4);
	sc->sc_enaddr[3] |= *p++ & 0x0f;
	sc->sc_enaddr[4] = (*p++ << 4);
	sc->sc_enaddr[4] |= *p++ & 0x0f;
	sc->sc_enaddr[5] = (*p++ << 4);
	sc->sc_enaddr[5] |= *p++ & 0x0f;

	sc->sc_copytodesc = lance_copytobuf_contig;
	sc->sc_copyfromdesc = lance_copyfrombuf_contig;
	sc->sc_copytobuf = lance_copytobuf_contig;
	sc->sc_copyfrombuf = lance_copyfrombuf_contig;
	sc->sc_zerobuf = lance_zerobuf_contig;

	sc->sc_rdcsr = lerdcsr;
	sc->sc_wrcsr = lewrcsr;
	sc->sc_hwinit = NULL;

	am7990_config(&lesc->sc_am7990);
}

int
leintr(int unit)
{
	struct am7990_softc *sc;

	if (unit >= le_cd.cd_ndevs)
		return 0;

	sc = le_cd.cd_devs[unit];
	return am7990_intr(sc);
}
