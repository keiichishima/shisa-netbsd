/* $NetBSD: ug.c,v 1.2 2007/05/08 17:17:14 xtraeme Exp $ */

/*
 * Copyright (c) 2007 Mihai Chelaru <kefren@netbsd.ro>
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

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: ug.c,v 1.2 2007/05/08 17:17:14 xtraeme Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/device.h>
#include <sys/malloc.h>
#include <sys/errno.h>
#include <sys/conf.h>
#include <sys/envsys.h>
#include <sys/time.h>

#include <machine/bus.h>
#include <machine/intr.h>

#include <dev/isa/isareg.h>
#include <dev/isa/isavar.h>

#include <dev/sysmon/sysmonvar.h>

#include <dev/ic/ugreg.h>
#include <dev/ic/ugvar.h>

uint8_t ug_ver;

/*
 * Imported from linux driver
 */

struct ug2_motherboard_info ug2_mb[] = {
	{ 0x000C, "unknown. Please send-pr(1)", {
		{ "CPU Core", 0, 0, 10, 1, 0 },
		{ "DDR", 1, 0, 10, 1, 0 },
		{ "DDR VTT", 2, 0, 10, 1, 0 },
		{ "CPU VTT 1.2V", 3, 0, 10, 1, 0 },
		{ "MCH & PCIE 1.5V", 4, 0, 10, 1, 0 },
		{ "MCH 2.5V", 5, 0, 20, 1, 0 },
		{ "ICH 1.05V", 6, 0, 10, 1, 0 },
		{ "ATX +12V (24-Pin)", 7, 0, 60, 1, 0 },
		{ "ATX +12V (4-pin)", 8, 0, 60, 1, 0 },
		{ "ATX +5V", 9, 0, 30, 1, 0 },
		{ "+3.3V", 10, 0, 20, 1, 0 },
		{ "5VSB", 11, 0, 30, 1, 0 },
		{ "CPU", 24, 1, 1, 1, 0 },
		{ "System", 25, 1, 1, 1, 0 },
		{ "PWM", 26, 1, 1, 1, 0 },
		{ "CPU Fan", 32, 2, 60, 1, 0 },
		{ "NB Fan", 33, 2, 60, 1, 0 },
		{ "SYS FAN", 34, 2, 60, 1, 0 },
		{ "AUX1 Fan", 35, 2, 60, 1, 0 },
		{ NULL, 0, 0, 0, 0, 0 } }
	},
	{ 0x000D, "Abit AW8", {
		{ "CPU Core", 0, 0, 10, 1, 0 },
		{ "DDR", 1, 0, 10, 1, 0 },
		{ "DDR VTT", 2, 0, 10, 1, 0 },
		{ "CPU VTT 1.2V", 3, 0, 10, 1, 0 },
		{ "MCH & PCIE 1.5V", 4, 0, 10, 1, 0 },
		{ "MCH 2.5V", 5, 0, 20, 1, 0 },
		{ "ICH 1.05V", 6, 0, 10, 1, 0 },
		{ "ATX +12V (24-Pin)", 7, 0, 60, 1, 0 },
		{ "ATX +12V (4-pin)", 8, 0, 60, 1, 0 },
		{ "ATX +5V", 9, 0, 30, 1, 0 },
		{ "+3.3V", 10, 0, 20, 1, 0 },
		{ "5VSB", 11, 0, 30, 1, 0 },
		{ "CPU", 24, 1, 1, 1, 0 },
		{ "System", 25, 1, 1, 1, 0 },
		{ "PWM1", 26, 1, 1, 1, 0 },
		{ "PWM2", 27, 1, 1, 1, 0 },
		{ "PWM3", 28, 1, 1, 1, 0 },
		{ "PWM4", 29, 1, 1, 1, 0 },
		{ "CPU Fan", 32, 2, 60, 1, 0 },
		{ "NB Fan", 33, 2, 60, 1, 0 },
		{ "SYS Fan", 34, 2, 60, 1, 0 },
		{ "AUX1 Fan", 35, 2, 60, 1, 0 },
		{ "AUX2 Fan", 36, 2, 60, 1, 0 },
		{ "AUX3 Fan", 37, 2, 60, 1, 0 },
		{ "AUX4 Fan", 38, 2, 60, 1, 0 },
		{ "AUX5 Fan", 39, 2, 60, 1, 0 },
		{ NULL, 0, 0, 0, 0, 0 } }
	},
	{ 0x000E, "Abit AL8", {
		{ "CPU Core", 0, 0, 10, 1, 0 },
		{ "DDR", 1, 0, 10, 1, 0 },
		{ "DDR VTT", 2, 0, 10, 1, 0 },
		{ "CPU VTT 1.2V", 3, 0, 10, 1, 0 },
		{ "MCH & PCIE 1.5V", 4, 0, 10, 1, 0 },
		{ "MCH 2.5V", 5, 0, 20, 1, 0 },
		{ "ICH 1.05V", 6, 0, 10, 1, 0 },
		{ "ATX +12V (24-Pin)", 7, 0, 60, 1, 0 },
		{ "ATX +12V (4-pin)", 8, 0, 60, 1, 0 },
		{ "ATX +5V", 9, 0, 30, 1, 0 },
		{ "+3.3V", 10, 0, 20, 1, 0 },
		{ "5VSB", 11, 0, 30, 1, 0 },
		{ "CPU", 24, 1, 1, 1, 0 },
		{ "System", 25, 1, 1, 1, 0 },
		{ "PWM", 26, 1, 1, 1, 0 },
		{ "CPU Fan", 32, 2, 60, 1, 0 },
		{ "NB Fan", 33, 2, 60, 1, 0 },
		{ "SYS Fan", 34, 2, 60, 1, 0 },
		{ NULL, 0, 0, 0, 0, 0 } }
	},
	{ 0x000F, "unknown. Please send-pr(1)", {
		{ "CPU Core", 0, 0, 10, 1, 0 },
		{ "DDR", 1, 0, 10, 1, 0 },
		{ "DDR VTT", 2, 0, 10, 1, 0 },
		{ "CPU VTT 1.2V", 3, 0, 10, 1, 0 },
		{ "MCH & PCIE 1.5V", 4, 0, 10, 1, 0 },
		{ "MCH 2.5V", 5, 0, 20, 1, 0 },
		{ "ICH 1.05V", 6, 0, 10, 1, 0 },
		{ "ATX +12V (24-Pin)", 7, 0, 60, 1, 0 },
		{ "ATX +12V (4-pin)", 8, 0, 60, 1, 0 },
		{ "ATX +5V", 9, 0, 30, 1, 0 },
		{ "+3.3V", 10, 0, 20, 1, 0 },
		{ "5VSB", 11, 0, 30, 1, 0 },
		{ "CPU", 24, 1, 1, 1, 0 },
		{ "System", 25, 1, 1, 1, 0 },
		{ "PWM", 26, 1, 1, 1, 0 },
		{ "CPU Fan", 32, 2, 60, 1, 0 },
		{ "NB Fan", 33, 2, 60, 1, 0 },
		{ "SYS Fan", 34, 2, 60, 1, 0 },
		{ NULL, 0, 0, 0, 0, 0 } }
	},
	{ 0x0010, "Abit NI8 SLI GR", {
		{ "CPU Core", 0, 0, 10, 1, 0 },
		{ "DDR", 1, 0, 10, 1, 0 },
		{ "DDR VTT", 2, 0, 10, 1, 0 },
		{ "CPU VTT 1.2V", 3, 0, 10, 1, 0 },
		{ "NB 1.4V", 4, 0, 10, 1, 0 },
		{ "SB 1.5V", 6, 0, 10, 1, 0 },
		{ "ATX +12V (24-Pin)", 7, 0, 60, 1, 0 },
		{ "ATX +12V (4-pin)", 8, 0, 60, 1, 0 },
		{ "ATX +5V", 9, 0, 30, 1, 0 },
		{ "+3.3V", 10, 0, 20, 1, 0 },
		{ "5VSB", 11, 0, 30, 1, 0 },
		{ "CPU", 24, 1, 1, 1, 0 },
		{ "SYS", 25, 1, 1, 1, 0 },
		{ "PWM", 26, 1, 1, 1, 0 },
		{ "CPU Fan", 32, 2, 60, 1, 0 },
		{ "NB Fan", 33, 2, 60, 1, 0 },
		{ "SYS Fan", 34, 2, 60, 1, 0 },
		{ "AUX1 Fan", 35, 2, 60, 1, 0 },
		{ "OTES1 Fan", 36, 2, 60, 1, 0 },
		{ NULL, 0, 0, 0, 0, 0 } }
	},
	{ 0x0011, "Abit AT8 32X", {
		{ "CPU Core", 0, 0, 10, 1, 0 },
		{ "DDR", 1, 0, 20, 1, 0 },
		{ "DDR VTT", 2, 0, 10, 1, 0 },
		{ "CPU VDDA 2.5V", 6, 0, 20, 1, 0 },
		{ "NB 1.8V", 4, 0, 10, 1, 0 },
		{ "NB 1.8V Dual", 5, 0, 10, 1, 0 },
		{ "HTV 1.2", 3, 0, 10, 1, 0 },
		{ "PCIE 1.2V", 12, 0, 10, 1, 0 },
		{ "NB 1.2V", 13, 0, 10, 1, 0 },
		{ "ATX +12V (24-Pin)", 7, 0, 60, 1, 0 },
		{ "ATX +12V (4-pin)", 8, 0, 60, 1, 0 },
		{ "ATX +5V", 9, 0, 30, 1, 0 },
		{ "+3.3V", 10, 0, 20, 1, 0 },
		{ "5VSB", 11, 0, 30, 1, 0 },
		{ "CPU", 24, 1, 1, 1, 0 },
		{ "NB", 25, 1, 1, 1, 0 },
		{ "System", 26, 1, 1, 1, 0 },
		{ "PWM", 27, 1, 1, 1, 0 },
		{ "CPU Fan", 32, 2, 60, 1, 0 },
		{ "NB Fan", 33, 2, 60, 1, 0 },
		{ "SYS Fan", 34, 2, 60, 1, 0 },
		{ "AUX1 Fan", 35, 2, 60, 1, 0 },
		{ "AUX2 Fan", 36, 2, 60, 1, 0 },
		{ NULL, 0, 0, 0, 0, 0 } }
	},
	{ 0x0012, "unknown. Please send-pr(1)", {
		{ "CPU Core", 0, 0, 10, 1, 0 },
		{ "DDR", 1, 0, 20, 1, 0 },
		{ "DDR VTT", 2, 0, 10, 1, 0 },
		{ "HyperTransport", 3, 0, 10, 1, 0 },
		{ "CPU VDDA 2.5V", 5, 0, 20, 1, 0 },
		{ "NB", 4, 0, 10, 1, 0 },
		{ "SB", 6, 0, 10, 1, 0 },
		{ "ATX +12V (24-Pin)", 7, 0, 60, 1, 0 },
		{ "ATX +12V (4-pin)", 8, 0, 60, 1, 0 },
		{ "ATX +5V", 9, 0, 30, 1, 0 },
		{ "+3.3V", 10, 0, 20, 1, 0 },
		{ "5VSB", 11, 0, 30, 1, 0 },
		{ "CPU", 24, 1, 1, 1, 0 },
		{ "SYS", 25, 1, 1, 1, 0 },
		{ "PWM", 26, 1, 1, 1, 0 },
		{ "CPU Fan", 32, 2, 60, 1, 0 },
		{ "NB Fan", 33, 2, 60, 1, 0 },
		{ "SYS Fan", 34, 2, 60, 1, 0 },
		{ "AUX1 Fan", 36, 2, 60, 1, 0 },
		{ NULL, 0, 0, 0, 0, 0 } }
	},
	{ 0x0013, "unknown. Please send-pr(1)", {
		{ "CPU Core", 0, 0, 10, 1, 0 },
		{ "DDR", 1, 0, 10, 1, 0 },
		{ "DDR VTT", 2, 0, 10, 1, 0 },
		{ "CPU VTT 1.2V", 3, 0, 10, 1, 0 },
		{ "MCH & PCIE 1.5V", 4, 0, 10, 1, 0 },
		{ "MCH 2.5V", 5, 0, 20, 1, 0 },
		{ "ICH 1.05V", 6, 0, 10, 1, 0 },
		{ "ATX +12V (24-Pin)", 7, 0, 60, 1, 0 },
		{ "ATX +12V (4-pin)", 8, 0, 60, 1, 0 },
		{ "ATX +5V", 9, 0, 30, 1, 0 },
		{ "+3.3V", 10, 0, 20, 1, 0 },
		{ "5VSB", 11, 0, 30, 1, 0 },
		{ "CPU", 24, 1, 1, 1, 0 },
		{ "System", 25, 1, 1, 1, 0 },
		{ "PWM1", 26, 1, 1, 1, 0 },
		{ "PWM2", 27, 1, 1, 1, 0 },
		{ "PWM3", 28, 1, 1, 1, 0 },
		{ "PWM4", 29, 1, 1, 1, 0 },
		{ "CPU Fan", 32, 2, 60, 1, 0 },
		{ "NB Fan", 33, 2, 60, 1, 0 },
		{ "SYS Fan", 34, 2, 60, 1, 0 },
		{ "AUX1 Fan", 35, 2, 60, 1, 0 },
		{ "AUX2 Fan", 36, 2, 60, 1, 0 },
		{ "AUX3 Fan", 37, 2, 60, 1, 0 },
		{ "AUX4 Fan", 38, 2, 60, 1, 0 },
		{ NULL, 0, 0, 0, 0, 0 } }
	},
	{ 0x0014, "Abit AB9 Pro", {
		{ "CPU Core", 0, 0, 10, 1, 0 },
		{ "DDR", 1, 0, 10, 1, 0 },
		{ "DDR VTT", 2, 0, 10, 1, 0 },
		{ "CPU VTT 1.2V", 3, 0, 10, 1, 0 },
		{ "MCH & PCIE 1.5V", 4, 0, 10, 1, 0 },
		{ "MCH 2.5V", 5, 0, 20, 1, 0 },
		{ "ICH 1.05V", 6, 0, 10, 1, 0 },
		{ "ATX +12V (24-Pin)", 7, 0, 60, 1, 0 },
		{ "ATX +12V (4-pin)", 8, 0, 60, 1, 0 },
		{ "ATX +5V", 9, 0, 30, 1, 0 },
		{ "+3.3V", 10, 0, 20, 1, 0 },
		{ "5VSB", 11, 0, 30, 1, 0 },
		{ "CPU", 24, 1, 1, 1, 0 },
		{ "System", 25, 1, 1, 1, 0 },
		{ "PWM", 26, 1, 1, 1, 0 },
		{ "CPU Fan", 32, 2, 60, 1, 0 },
		{ "NB Fan", 33, 2, 60, 1, 0 },
		{ "SYS Fan", 34, 2, 60, 1, 0 },
		{ NULL, 0, 0, 0, 0, 0 } }
	},
	{ 0x0015, "unknown. Please send-pr(1)", {
		{ "CPU Core", 0, 0, 10, 1, 0 },
		{ "DDR", 1, 0, 20, 1, 0 },
		{ "DDR VTT", 2, 0, 10, 1, 0 },
		{ "HyperTransport", 3, 0, 10, 1, 0 },
		{ "CPU VDDA 2.5V", 5, 0, 20, 1, 0 },
		{ "NB", 4, 0, 10, 1, 0 },
		{ "SB", 6, 0, 10, 1, 0 },
		{ "ATX +12V (24-Pin)", 7, 0, 60, 1, 0 },
		{ "ATX +12V (4-pin)", 8, 0, 60, 1, 0 },
		{ "ATX +5V", 9, 0, 30, 1, 0 },
		{ "+3.3V", 10, 0, 20, 1, 0 },
		{ "5VSB", 11, 0, 30, 1, 0 },
		{ "CPU", 24, 1, 1, 1, 0 },
		{ "SYS", 25, 1, 1, 1, 0 },
		{ "PWM", 26, 1, 1, 1, 0 },
		{ "CPU Fan", 32, 2, 60, 1, 0 },
		{ "NB Fan", 33, 2, 60, 1, 0 },
		{ "SYS Fan", 34, 2, 60, 1, 0 },
		{ "AUX1 Fan", 33, 2, 60, 1, 0 },
		{ "AUX2 Fan", 35, 2, 60, 1, 0 },
		{ "AUX3 Fan", 36, 2, 60, 1, 0 },
		{ NULL, 0, 0, 0, 0, 0 } }
	},
	{ 0x0016, "generic", {
		{ "CPU Core", 0, 0, 10, 1, 0 },
		{ "DDR", 1, 0, 20, 1, 0 },
		{ "DDR VTT", 2, 0, 10, 1, 0 },
		{ "CPU VTT 1.2V", 3, 0, 10, 1, 0 },
		{ "MCH & PCIE 1.5V", 4, 0, 10, 1, 0 },
		{ "MCH 2.5V", 5, 0, 20, 1, 0 },
		{ "ICH 1.05V", 6, 0, 10, 1, 0 },
		{ "ATX +12V (24-Pin)", 7, 0, 60, 1, 0 },
		{ "ATX +12V (4-pin)", 8, 0, 60, 1, 0 },
		{ "ATX +5V", 9, 0, 30, 1, 0 },
		{ "+3.3V", 10, 0, 20, 1, 0 },
		{ "5VSB", 11, 0, 30, 1, 0 },
		{ "CPU", 24, 1, 1, 1, 0 },
		{ "System", 25, 1, 1, 1, 0 },
		{ "PWM", 26, 1, 1, 1, 0 },
		{ "CPU Fan", 32, 2, 60, 1, 0 },
		{ "NB Fan", 33, 2, 60, 1, 0 },
		{ "SYS FAN", 34, 2, 60, 1, 0 },
		{ "AUX1 Fan", 35, 2, 60, 1, 0 },
		{ NULL, 0, 0, 0, 0, 0 } }
	},
	{ 0x0000, NULL, { { NULL, 0, 0, 0, 0, 0 } } }
};


int
ug_reset(struct ug_softc *sc)
{
	int cnt = 0;

	while (bus_space_read_1(sc->sc_iot, sc->sc_ioh, UG_DATA) != 0x08) {
	/* 8 meaning Voodoo */
               
		if (cnt++ > UG_DELAY_CYCLES)
			return 0;

		bus_space_write_1(sc->sc_iot, sc->sc_ioh, UG_DATA, 0);

		/* Wait for 0x09 at Data Port */
		if (!ug_waitfor(sc, UG_DATA, 0x09))
			return 0;
               
		/* Wait for 0xAC at Cmd Port */
		if (!ug_waitfor(sc, UG_CMD, 0xAC))
			return 0;
	}

	return 1;
}

uint8_t
ug_read(struct ug_softc *sc, unsigned short sensor)
{
	uint8_t bank, sens, rv;

	bank = (sensor & 0xFF00) >> 8;
	sens = sensor & 0x00FF;

	bus_space_write_1(sc->sc_iot, sc->sc_ioh, UG_DATA, bank);

	/* Wait 8 at Data Port */
	if (!ug_waitfor(sc, UG_DATA, 8))
		return 0;

	bus_space_write_1(sc->sc_iot, sc->sc_ioh, UG_CMD, sens);

	/* Wait 1 at Data Port */
	if (!ug_waitfor(sc, UG_DATA, 1))
		return 0;

	/* Finally read the sensor */
	rv = bus_space_read_1(sc->sc_iot, sc->sc_ioh, UG_CMD);

	ug_reset(sc);

	return rv;
}

int
ug_waitfor(struct ug_softc *sc, uint16_t offset, uint8_t value)
{
	int cnt = 0;
	while (bus_space_read_1(sc->sc_iot, sc->sc_ioh, offset) != value) {
		if (cnt++ > UG_DELAY_CYCLES)
			return 0;
	}
	return 1;
}

void
ug_setup_sensors(struct ug_softc *sc)
{
	int i;

	/* Setup Temps */
	for (i = 0; i < UG_VOLT_MIN; i++)
		sc->sc_data[i].units = sc->sc_info[i].units = ENVSYS_STEMP;

#define COPYDESCR(x, y)				\
	do {					\
		strlcpy((x), (y), sizeof(x));	\
	} while (0)

	COPYDESCR(sc->sc_info[0].desc, "CPU Temp");
	COPYDESCR(sc->sc_info[1].desc, "SYS Temp");
	COPYDESCR(sc->sc_info[2].desc, "PWN Temp");

	/* Right, Now setup U sensors */

	for (i = UG_VOLT_MIN; i < UG_FAN_MIN; i++) {
		sc->sc_data[i].units = sc->sc_info[i].units = ENVSYS_SVOLTS_DC;
		sc->sc_info[i].rfact = UG_RFACT;
	}

	COPYDESCR(sc->sc_info[3].desc, "HTVdd");
	COPYDESCR(sc->sc_info[4].desc, "VCore");
	COPYDESCR(sc->sc_info[5].desc, "DDRVdd");
	COPYDESCR(sc->sc_info[6].desc, "Vdd3V3");
	COPYDESCR(sc->sc_info[7].desc, "Vdd5V");
	COPYDESCR(sc->sc_info[8].desc, "NBVdd");
	COPYDESCR(sc->sc_info[9].desc, "AGPVdd");
	COPYDESCR(sc->sc_info[10].desc, "DDRVtt");
	COPYDESCR(sc->sc_info[11].desc, "Vdd5VSB");
	COPYDESCR(sc->sc_info[12].desc, "Vdd3VDual");
	COPYDESCR(sc->sc_info[13].desc, "SBVdd");

	/* Fan sensors */
	for (i = UG_FAN_MIN; i < UG_NUM_SENSORS; i++)
		sc->sc_data[i].units = sc->sc_info[i].units = ENVSYS_SFANRPM;

	COPYDESCR(sc->sc_info[14].desc, "CPU Fan");
	COPYDESCR(sc->sc_info[15].desc, "NB Fan");
	COPYDESCR(sc->sc_info[16].desc, "SYS Fan");
	COPYDESCR(sc->sc_info[17].desc, "AUX Fan 1");
	COPYDESCR(sc->sc_info[18].desc, "AUX Fan 2");
}

int
ug_gtredata(struct sysmon_envsys *sme, envsys_tre_data_t *tred)
{
	struct ug_softc *sc = sme->sme_cookie;
	envsys_tre_data_t *t = sc->sc_data;	/* For easier read */

	/* Sensors return C while we need uK */

	if (tred->sensor < UG_VOLT_MIN - 1) /* CPU and SYS Temps */
		t[tred->sensor].cur.data_us = ug_read(sc, UG_CPUTEMP +
		    tred->sensor) * 1000000 + 273150000;
	else if (tred->sensor == 2) /* PWMTEMP */
		t[tred->sensor].cur.data_us = ug_read(sc, UG_PWMTEMP)
		    * 1000000 + 273150000;

	/* Voltages */

#define VOLT_SENSOR	UG_HTV + tred->sensor - UG_VOLT_MIN

	else
	    if ((tred->sensor >= UG_VOLT_MIN) && (tred->sensor < UG_FAN_MIN)) {
		t[tred->sensor].cur.data_s = ug_read(sc, VOLT_SENSOR);
		switch(VOLT_SENSOR) {
			case UG_5V:		/* 6V RFact */
			case UG_5VSB:
				t[tred->sensor].cur.data_s *= UG_RFACT6;
				break;
			case UG_3V3:		/* 4V RFact */
			case UG_3VDUAL:
				t[tred->sensor].cur.data_s *= UG_RFACT4;
				break;
			default:		/* 3V RFact */
				t[tred->sensor].cur.data_s *= UG_RFACT3;
		}
	    } else

#undef VOLT_SENSOR

	/* and Fans */
	if (tred->sensor >= UG_FAN_MIN)
		t[tred->sensor].cur.data_s = ug_read(sc, UG_CPUFAN +
		    tred->sensor - UG_FAN_MIN) * UG_RFACT_FAN;
	else
		return ENODEV;		/* should I scream and panic ? */

	*tred = sc->sc_data[tred->sensor];
	return 0;
}

int
ug_streinfo_ni(struct sysmon_envsys *sme, envsys_basic_info_t *binfo)
{
	/* not implemented */
	binfo->validflags = 0;

	return 0;
}

void
ug2_attach(struct ug_softc *sc)
{
	uint8_t buf[2];
	int i, i2;
	struct ug2_motherboard_info *ai;
	struct ug2_sensor_info *si;
	struct envsys_range ug2_ranges[7];	/* XXX: why only 7 ?! */

	aprint_normal(": Abit uGuru 2005 system monitor\n");

	memcpy(ug2_ranges, ug_ranges, 7 * sizeof(struct envsys_range));

	for (i = 0; i < 7; i++)
		ug2_ranges[i].low = ug2_ranges[i].high = 0xFF;

	if (ug2_read(sc, UG2_MISC_BANK, UG2_BOARD_ID, 2, buf) != 2) {
		aprint_error("%s: Cannot detect board ID. Using default\n",
			sc->sc_dev.dv_xname);
		buf[0] = UG_MAX_MSB_BOARD;
		buf[1] = UG_MAX_LSB_BOARD;
	}

	if (buf[0] > UG_MAX_MSB_BOARD || buf[1] > UG_MAX_LSB_BOARD ||
		buf[1] < UG_MIN_LSB_BOARD) {
		aprint_error("%s: Invalid board ID(%X,%X). Using default\n",
			sc->sc_dev.dv_xname, buf[0], buf[1]);
		buf[0] = UG_MAX_MSB_BOARD;
		buf[1] = UG_MAX_LSB_BOARD;
	}

	ai = &ug2_mb[buf[1] - UG_MIN_LSB_BOARD];

	aprint_normal("%s: mainboard %s (%.2X%.2X)\n", sc->sc_dev.dv_xname,
	    ai->name, buf[0], buf[1]);

	sc->mbsens = (void*)ai->sensors;

	for (i = 0, si = ai->sensors; si && si->name; si++, i++) {
		COPYDESCR(sc->sc_info[i].desc, si->name);
		sc->sc_data[i].sensor = sc->sc_info[i].sensor = i;
		sc->sc_data[i].validflags = (ENVSYS_FVALID|ENVSYS_FCURVALID);
		sc->sc_info[i].validflags = ENVSYS_FVALID;
		sc->sc_data[i].warnflags = ENVSYS_WARN_OK;
		sc->sc_info[i].rfact = 1;
		switch (si->type) {
			case UG2_VOLTAGE_SENSOR:
				sc->sc_data[i].units = sc->sc_info[i].units = 
					ENVSYS_SVOLTS_DC;
				sc->sc_info[i].rfact = UG_RFACT;
				ug2_ranges[3].high = i;
				if (ug2_ranges[3].low == 0xFF)
					ug2_ranges[3].low = i;
				break;
			case UG2_TEMP_SENSOR:
				sc->sc_data[i].units = sc->sc_info[i].units =
					ENVSYS_STEMP;
				ug2_ranges[0].high = i;
				if (ug2_ranges[0].low == 0xFF)
					ug2_ranges[0].low = i;
				break;
			case UG2_FAN_SENSOR:
				sc->sc_data[i].units = sc->sc_info[i].units =
					ENVSYS_SFANRPM;
				ug2_ranges[1].high = i;
				if (ug2_ranges[0].low == 0xFF)
					ug2_ranges[0].low = i;
		}
	}
#undef COPYDESCR

	for (i2 = 0; i2 < 7; i2++)
		if (ug2_ranges[i2].low == 0xFF ||
		    ug2_ranges[i2].high == 0xFF) {
			ug2_ranges[i2].low = 1;
			ug2_ranges[i2].high = 0;
		}

	sc->sc_sysmon.sme_ranges = ug2_ranges;
	sc->sc_sysmon.sme_sensor_info = sc->sc_info;
	sc->sc_sysmon.sme_sensor_data = sc->sc_data;
	sc->sc_sysmon.sme_cookie = sc;
	sc->sc_sysmon.sme_gtredata = ug2_gtredata;
	sc->sc_sysmon.sme_streinfo = ug_streinfo_ni;
	sc->sc_sysmon.sme_nsensors = i;
	sc->sc_sysmon.sme_envsys_version = UG_DRV_VERSION;
	sc->sc_sysmon.sme_flags = 0;

	if (sysmon_envsys_register(&sc->sc_sysmon))
		aprint_error("%s: unable to register with sysmon\n",
		    sc->sc_dev.dv_xname);
}

int
ug2_gtredata(struct sysmon_envsys *sme, envsys_tre_data_t *tred)
{
	struct ug_softc *sc = sme->sme_cookie;
	envsys_tre_data_t *t = sc->sc_data;	/* makes code readable */
	struct ug2_sensor_info *si = (struct ug2_sensor_info *)sc->mbsens;
	int rfact = 1;
	uint8_t v;

	if (tred->sensor >= sc->sc_sysmon.sme_nsensors)
		return ENODEV;	/* ?! */

	si += tred->sensor;

#define SENSOR_VALUE (v * si->multiplier * rfact / si->divisor + si->offset)

	if (ug2_read(sc, UG2_SENSORS_BANK, UG2_VALUES_OFFSET +
	    si->port, 1, &v) == 1) {
		switch (si->type) {
		case UG2_TEMP_SENSOR:
		    t[tred->sensor].cur.data_us = SENSOR_VALUE * 1000000
			+ 273150000;
		    break;
		case UG2_VOLTAGE_SENSOR:
		    rfact = UG_RFACT;
		    t[tred->sensor].cur.data_us = SENSOR_VALUE;
		    break;
		default:
		    t[tred->sensor].cur.data_s = SENSOR_VALUE;
		}
	} else
		return ENODEV;

#undef SENSOR_VALUE

	*tred = sc->sc_data[tred->sensor];
	return 0;
}

int
ug2_wait_ready(struct ug_softc *sc)
{
	int cnt = 0;

	bus_space_write_1(sc->sc_iot, sc->sc_ioh, UG_DATA, 0x1a);
	while (bus_space_read_1(sc->sc_iot, sc->sc_ioh, UG_DATA) &
	    UG2_STATUS_BUSY) {
		if (cnt++ > UG_DELAY_CYCLES)
			return 0;
	}
	return 1;
}

int
ug2_wait_readable(struct ug_softc *sc)
{
	int cnt = 0;

	while (!(bus_space_read_1(sc->sc_iot, sc->sc_ioh, UG_DATA) &
		UG2_STATUS_READY_FOR_READ)) {
		if (cnt++ > UG_DELAY_CYCLES)
			return 0;
	}
	return 1;
}

int
ug2_sync(struct ug_softc *sc)
{
	int cnt = 0;

#define UG2_WAIT_READY if(ug2_wait_ready(sc) == 0) return 0;

	/* Don't sync two times in a row */
	if(ug_ver != 0) {
		ug_ver = 0;
		return 1;
	}

	UG2_WAIT_READY;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, UG_DATA, 0x20);
	UG2_WAIT_READY;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, UG_CMD, 0x10);
	UG2_WAIT_READY;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, UG_CMD, 0x00);
	UG2_WAIT_READY;
	if (ug2_wait_readable(sc) == 0)
		return 0;
	while (bus_space_read_1(sc->sc_iot, sc->sc_ioh, UG_CMD) != 0xAC)
		if (cnt++ > UG_DELAY_CYCLES)
			return 0;
	return 1;
}

int
ug2_read(struct ug_softc *sc, uint8_t bank, uint8_t offset, uint8_t count,
	 uint8_t *ret)
{
	int i;

	if (ug2_sync(sc) == 0)
		return 0;

	bus_space_write_1(sc->sc_iot, sc->sc_ioh, UG_DATA, 0x1A);
	UG2_WAIT_READY;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, UG_CMD, bank);
	UG2_WAIT_READY;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, UG_CMD, offset);
	UG2_WAIT_READY;
	bus_space_write_1(sc->sc_iot, sc->sc_ioh, UG_CMD, count);
	UG2_WAIT_READY;

#undef UG2_WAIT_READY

	/* Now wait for the results */
	for (i = 0; i < count; i++) {
		if (ug2_wait_readable(sc) == 0)
			break;
		ret[i] = bus_space_read_1(sc->sc_iot, sc->sc_ioh, UG_CMD);
	}

	return i;
}
