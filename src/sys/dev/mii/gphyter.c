/*	$NetBSD: gphyter.c,v 1.15 2004/08/23 06:16:06 thorpej Exp $	*/

/*-
 * Copyright (c) 1998, 1999, 2000, 2001 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jason R. Thorpe of the Numerical Aerospace Simulation Facility,
 * NASA Ames Research Center.
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

/*
 * Copyright (c) 1997 Manuel Bouyer.  All rights reserved.
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
 *	This product includes software developed by Manuel Bouyer.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * driver for National Semiconductor's DP83891 and DP83861 `Gig PHYTER'
 * ethernet 10/100/1000 PHYs.  The DP83891 is an older, non-firmware-
 * driven version of the DP83861.
 *
 * Data Sheet available from www.national.com
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: gphyter.c,v 1.15 2004/08/23 06:16:06 thorpej Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/device.h>
#include <sys/socket.h>
#include <sys/errno.h>

#include <net/if.h>
#include <net/if_media.h>

#include <dev/mii/mii.h>
#include <dev/mii/miivar.h>
#include <dev/mii/miidevs.h>

#include <dev/mii/gphyterreg.h>

static int	gphytermatch(struct device *, struct cfdata *, void *);
static void	gphyterattach(struct device *, struct device *, void *);

CFATTACH_DECL(gphyter, sizeof(struct mii_softc),
    gphytermatch, gphyterattach, mii_phy_detach, mii_phy_activate);

static int	gphyter_service(struct mii_softc *, struct mii_data *, int);
static void	gphyter_status(struct mii_softc *);

static const struct mii_phy_funcs gphyter_funcs = {
	gphyter_service, gphyter_status, mii_phy_reset,
};

static const struct mii_phydesc gphyters[] = {
	{ MII_OUI_xxNATSEMI,		MII_MODEL_xxNATSEMI_DP83861,
	  MII_STR_xxNATSEMI_DP83861 },

	{ MII_OUI_xxNATSEMI,		MII_MODEL_xxNATSEMI_DP83891,
	  MII_STR_xxNATSEMI_DP83891 },

	{ 0,				0,
	  NULL },
};

static int
gphytermatch(struct device *parent, struct cfdata *match, void *aux)
{
	struct mii_attach_args *ma = aux;

	if (mii_phy_match(ma, gphyters) != NULL)
		return (10);

	return (0);
}

static void
gphyterattach(struct device *parent, struct device *self, void *aux)
{
	struct mii_softc *sc = (struct mii_softc *)self;
	struct mii_attach_args *ma = aux;
	struct mii_data *mii = ma->mii_data;
	const struct mii_phydesc *mpd;
	int anar, strap;

	mpd = mii_phy_match(ma, gphyters);
	aprint_naive(": Media interface\n");
	aprint_normal(": %s, rev. %d\n", mpd->mpd_name, MII_REV(ma->mii_id2));

	sc->mii_inst = mii->mii_instance;
	sc->mii_phy = ma->mii_phyno;
	sc->mii_funcs = &gphyter_funcs;
	sc->mii_pdata = mii;
	sc->mii_flags = ma->mii_flags;
	sc->mii_anegticks = 5;

	PHY_RESET(sc);

	sc->mii_capabilities =
	    PHY_READ(sc, MII_BMSR) & ma->mii_capmask;
	if (sc->mii_capabilities & BMSR_EXTSTAT)
		sc->mii_extcapabilities = PHY_READ(sc, MII_EXTSR);

	/*
	 * The Gig PHYTER seems to have the 10baseT BMSR bits
	 * hard-wired to 0, even though the device supports
	 * 10baseT.  What we do instead is read the post-reset
	 * ANAR, who's 10baseT-related bits are set by strapping
	 * pin 180, and fake the BMSR bits.
	 */
	anar = PHY_READ(sc, MII_ANAR);
	if (anar & ANAR_10)
		sc->mii_capabilities |= (BMSR_10THDX & ma->mii_capmask);
	if (anar & ANAR_10_FD)
		sc->mii_capabilities |= (BMSR_10TFDX & ma->mii_capmask);

	aprint_normal("%s: ", sc->mii_dev.dv_xname);
	if ((sc->mii_capabilities & BMSR_MEDIAMASK) == 0 &&
	    (sc->mii_extcapabilities & EXTSR_MEDIAMASK) == 0)
		aprint_error("no media present");
	else
		mii_phy_add_media(sc);
	aprint_normal("\n");

	strap = PHY_READ(sc, MII_GPHYTER_STRAP);
	aprint_normal("%s: strapped to %s mode", sc->mii_dev.dv_xname,
	    (strap & STRAP_MS_VAL) ? "master" : "slave");
	if (strap & STRAP_NC_MODE)
		aprint_normal(", pre-C5 BCM5400 compat enabled");
	aprint_normal("\n");
}

static int
gphyter_service(struct mii_softc *sc, struct mii_data *mii, int cmd)
{
	struct ifmedia_entry *ife = mii->mii_media.ifm_cur;
	int reg;

	switch (cmd) {
	case MII_POLLSTAT:
		/*
		 * If we're not polling our PHY instance, just return.
		 */
		if (IFM_INST(ife->ifm_media) != sc->mii_inst)
			return (0);
		break;

	case MII_MEDIACHG:
		/*
		 * If the media indicates a different PHY instance,
		 * isolate ourselves.
		 */
		if (IFM_INST(ife->ifm_media) != sc->mii_inst) {
			reg = PHY_READ(sc, MII_BMCR);
			PHY_WRITE(sc, MII_BMCR, reg | BMCR_ISO);
			return (0);
		}

		/*
		 * If the interface is not up, don't do anything.
		 */
		if ((mii->mii_ifp->if_flags & IFF_UP) == 0)
			break;

		mii_phy_setmedia(sc);
		break;

	case MII_TICK:
		/*
		 * If we're not currently selected, just return.
		 */
		if (IFM_INST(ife->ifm_media) != sc->mii_inst)
			return (0);

		if (mii_phy_tick(sc) == EJUSTRETURN)
			return (0);
		break;

	case MII_DOWN:
		mii_phy_down(sc);
		return (0);
	}

	/* Update the media status. */
	mii_phy_status(sc);

	/* Callback if something changed. */
	mii_phy_update(sc, cmd);
	return (0);
}

static void
gphyter_status(struct mii_softc *sc)
{
	struct mii_data *mii = sc->mii_pdata;
	struct ifmedia_entry *ife = mii->mii_media.ifm_cur;
	int bmsr, bmcr, physup, gtsr;

	mii->mii_media_status = IFM_AVALID;
	mii->mii_media_active = IFM_ETHER;

	bmsr = PHY_READ(sc, MII_BMSR) | PHY_READ(sc, MII_BMSR);

	physup = PHY_READ(sc, MII_GPHYTER_PHY_SUP);

	if (physup & PHY_SUP_LINK)
		mii->mii_media_status |= IFM_ACTIVE;

	bmcr = PHY_READ(sc, MII_BMCR);
	if (bmcr & BMCR_ISO) {
		mii->mii_media_active |= IFM_NONE;
		mii->mii_media_status = 0;
		return;
	}

	if (bmcr & BMCR_LOOP)
		mii->mii_media_active |= IFM_LOOP;

	if (bmcr & BMCR_AUTOEN) {
		/*
		 * The media status bits are only valid of autonegotiation
		 * has completed (or it's disabled).
		 */
		if ((bmsr & BMSR_ACOMP) == 0) {
			/* Erg, still trying, I guess... */
			mii->mii_media_active |= IFM_NONE;
			return;
		}

		switch (physup & (PHY_SUP_SPEED1|PHY_SUP_SPEED0)) {
		case PHY_SUP_SPEED1:
			mii->mii_media_active |= IFM_1000_T;
			gtsr = PHY_READ(sc, MII_100T2SR);
			if (gtsr & GTSR_MS_RES)
				mii->mii_media_active |= IFM_ETH_MASTER;
			break;

		case PHY_SUP_SPEED0:
			mii->mii_media_active |= IFM_100_TX;
			break;

		case 0:
			mii->mii_media_active |= IFM_10_T;
			break;

		default:
			mii->mii_media_active |= IFM_NONE;
			mii->mii_media_status = 0;
		}
		if (physup & PHY_SUP_DUPLEX)
			mii->mii_media_active |=
			    IFM_FDX | mii_phy_flowstatus(sc);
	} else
		mii->mii_media_active = ife->ifm_media;
}
