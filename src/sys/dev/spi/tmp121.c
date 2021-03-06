/* $NetBSD: tmp121.c,v 1.1 2006/10/02 07:18:19 gdamore Exp $ */

/*-
 * Copyright (c) 2006 Urbana-Champaign Independent Media Center.
 * Copyright (c) 2006 Garrett D'Amore.
 * All rights reserved.
 *
 * Portions of this code were written by Garrett D'Amore for the
 * Champaign-Urbana Community Wireless Network Project.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgements:
 *      This product includes software developed by the Urbana-Champaign
 *      Independent Media Center.
 *	This product includes software developed by Garrett D'Amore.
 * 4. Urbana-Champaign Independent Media Center's name and Garrett
 *    D'Amore's name may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE URBANA-CHAMPAIGN INDEPENDENT
 * MEDIA CENTER AND GARRETT D'AMORE ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE URBANA-CHAMPAIGN INDEPENDENT
 * MEDIA CENTER OR GARRETT D'AMORE BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: tmp121.c,v 1.1 2006/10/02 07:18:19 gdamore Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/kernel.h>

#include <dev/sysmon/sysmonvar.h>

#include <dev/spi/spivar.h>

struct tmp121temp_softc {
	struct device sc_dev;

	struct spi_handle *sc_sh;
	
	struct envsys_tre_data sc_sensor[1];
	struct envsys_basic_info sc_info[1];

	struct sysmon_envsys sc_sysmon;
};

static int tmp121temp_match(struct device *, struct cfdata *, void *);
static void tmp121temp_attach(struct device *, struct device *, void *);

static int tmp121temp_gtredata(struct sysmon_envsys *,
    struct envsys_tre_data *);
static int tmp121temp_streinfo(struct sysmon_envsys *,
    struct envsys_basic_info *);

CFATTACH_DECL(tmp121temp, sizeof(struct tmp121temp_softc),
    tmp121temp_match, tmp121temp_attach, NULL, NULL);


static const struct envsys_range tmp121temp_ranges[] = {
	/* this looks suspect to me, manual says ranges are inclusive */
	{ 0, 1,	ENVSYS_STEMP },
	{ 1, 0,	-1 },
};

static int
tmp121temp_match(struct device *parent, struct cfdata *cf, void *aux)
{
	struct spi_attach_args *sa = aux;

	/* configure for 10MHz */
	if (spi_configure(sa->sa_handle, SPI_MODE_0, 1000000))
		return 0;

	return 1;
}

static void
tmp121temp_attach(struct device *parent, struct device *self, void *aux)
{
	struct tmp121temp_softc *sc = device_private(self);
	struct spi_attach_args *sa = aux;
	prop_string_t desc;

	aprint_naive(": Temperature Sensor\n");	
	aprint_normal(": TI TMP121 Temperature Sensor\n");

	sc->sc_sh = sa->sa_handle;

	sc->sc_info[0].sensor = 0;
	sc->sc_info[0].validflags = ENVSYS_FVALID;

	sc->sc_sensor[0].sensor = 0;
	sc->sc_sensor[0].validflags = ENVSYS_FVALID;
	sc->sc_sensor[0].warnflags = ENVSYS_WARN_OK;
	sc->sc_sensor[0].units = sc->sc_info[0].units = ENVSYS_STEMP;

	/*
	 * set up the description, which we allow MD code to override in
	 * a property.
	 */
	desc = prop_dictionary_get(device_properties(self),
	    "envsys-description");
	if (desc != NULL &&
	    prop_object_type(desc) == PROP_TYPE_STRING &&
	    prop_string_size(desc) > 0)
		strcpy(sc->sc_info[0].desc, prop_string_cstring_nocopy(desc));
	else
		strcpy(sc->sc_info[0].desc, device_xname(self));

	
	sc->sc_sysmon.sme_ranges = tmp121temp_ranges;
	sc->sc_sysmon.sme_sensor_info = sc->sc_info;
	sc->sc_sysmon.sme_sensor_data = sc->sc_sensor;
	sc->sc_sysmon.sme_gtredata = tmp121temp_gtredata;
	sc->sc_sysmon.sme_streinfo = tmp121temp_streinfo;
	sc->sc_sysmon.sme_cookie = sc;
	sc->sc_sysmon.sme_nsensors = 1;
	sc->sc_sysmon.sme_envsys_version = 1000;

	if (sysmon_envsys_register(&sc->sc_sysmon))
		aprint_error("%s: unable to register with sysmon\n",
		    device_xname(self));

}

static void
tmp121temp_refresh(struct tmp121temp_softc *sc)
{
	uint16_t		reg;
	int16_t			sreg;
	int			val;

	if (spi_recv(sc->sc_sh, 2, (uint8_t *)&reg) != 0) {
		sc->sc_sensor[0].validflags &= ~ENVSYS_FCURVALID;
		return;
	}

	sreg = (int16_t)be16toh(reg);

	/*
	 * convert to uK:
	 *
	 * TMP121 bits:
	 * D15		: sign bit
	 * D14-D3	: data (D14 is MSB)
	 * D2-D0	: zero
	 *
	 * The data is represented in units of 0.0625 deg C.
	 */
	sreg >>= 3;
	val = sreg * 62500 + 273150000;

	sc->sc_sensor[0].cur.data_us = val;
	sc->sc_sensor[0].validflags |= ENVSYS_FCURVALID;
}

static int
tmp121temp_gtredata(struct sysmon_envsys *sme, struct envsys_tre_data *tred)
{
	struct tmp121temp_softc *sc = sme->sme_cookie;

	/*
	 * XXX: locking?  possibly we could copy out bad sensor data.
	 * I'm not going to worry about it right now -- sysmon is probably
	 * single threaded anyway.
	 */

	tmp121temp_refresh(sc);
	*tred = sc->sc_sensor[tred->sensor];

	return (0);
}

static int
tmp121temp_streinfo(struct sysmon_envsys *sme, struct envsys_basic_info *binfo)
{
	struct tmp121temp_softc *sc = sme->sme_cookie;

	memcpy(sc->sc_info[binfo->sensor].desc, binfo->desc,
	    sizeof(sc->sc_info[binfo->sensor].desc));
	sc->sc_info[binfo->sensor].desc[
	    sizeof(sc->sc_info[binfo->sensor].desc) - 1] = '\0';

	binfo->validflags = ENVSYS_FVALID;

	return (0);
}
