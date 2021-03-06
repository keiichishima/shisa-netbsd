/*	$NetBSD: battery.c,v 1.1 2007/02/15 01:48:40 macallan Exp $ */

/*-
 * Copyright (c) 2007 Michael Lorenz
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
 * 3. Neither the name of The NetBSD Foundation nor the names of its
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
__KERNEL_RCSID(0, "$NetBSD: battery.c,v 1.1 2007/02/15 01:48:40 macallan Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/device.h>
#include <sys/proc.h>

#include <dev/sysmon/sysmonvar.h>
#include <dev/sysmon/sysmon_taskq.h>

#include <macppc/dev/pmuvar.h>
#include <macppc/dev/batteryvar.h>
#include <machine/bus.h>
#include "opt_battery.h"

#ifdef BATTERY_DEBUG
#define DPRINTF printf
#else
#define DPRINTF while (0) printf
#endif

#define BTYPE_COMET	1
#define BTYPE_HOOPER	2

#define BAT_CPU_TEMPERATURE	0
#define BAT_AC_PRESENT		1
#define BAT_PRESENT		2
#define BAT_VOLTAGE		3
#define BAT_CURRENT		4
#define BAT_MAX_CHARGE		5
#define BAT_CHARGE		6
#define BAT_CHARGING		7
#define BAT_DISCHARGING		8
#define BAT_FULL		9
#define BAT_TEMPERATURE		10
#define BAT_NSENSORS		11  /* number of sensors */

struct battery_softc {
	struct device sc_dev;
	struct pmu_ops *sc_pmu_ops;
	int sc_type;
	
	/* envsys stuff */
	struct sysmon_envsys sc_sysmon;
	struct envsys_basic_info sc_sinfo[BAT_NSENSORS];
	struct envsys_tre_data sc_sdata[BAT_NSENSORS];

	/* battery status */
	int sc_flags;
	int sc_voltage;
	int sc_charge;
	int sc_current;
	int sc_time;
	int sc_cpu_temp;
	int sc_bat_temp;
	int sc_vmax_charged;
	int sc_vmax_charging;
};

static void battery_attach(struct device *, struct device *, void *);
static int battery_match(struct device *, struct cfdata *, void *);
static int battery_update(struct battery_softc *, int);
static void battery_setup_envsys(struct battery_softc *);
static int battery_gtredata(struct sysmon_envsys *, struct envsys_tre_data *);
static int battery_streinfo(struct sysmon_envsys *,
    struct envsys_basic_info *);

CFATTACH_DECL(battery, sizeof(struct battery_softc),
    battery_match, battery_attach, NULL, NULL);

static int
battery_match(struct device *parent, struct cfdata *cf, void *aux)
{
	struct battery_attach_args *baa = aux;

	if (baa->baa_type == BATTERY_TYPE_LEGACY)
		return 1;

	return 0;
}

static void
battery_attach(struct device *parent, struct device *self, void *aux)
{
	struct battery_attach_args *baa = aux;
	struct battery_softc *sc = (struct battery_softc *)self;
	uint32_t reg;

	sc->sc_pmu_ops = baa->baa_pmu_ops;
	printf(": legacy battery ");

	reg = in32rb(0xf3000034);
	DPRINTF("reg: %08x\n", reg);
	if (reg & 0x20000000) {
		sc->sc_type = BTYPE_HOOPER;
		sc->sc_vmax_charged = 330;
		sc->sc_vmax_charging = 365;
		printf("[hooper]\n");
	} else {
		sc->sc_type = BTYPE_COMET;
		sc->sc_vmax_charged = 189;
		sc->sc_vmax_charging = 213;
		printf("[comet]\n");
	}
	battery_update(sc, 1);
	battery_setup_envsys(sc);
}

static int
battery_update(struct battery_softc *sc, int out)
{
	int len, vmax, pcharge, vb;
	uint8_t buf[16];

	len = sc->sc_pmu_ops->do_command(sc->sc_pmu_ops->cookie,
	    PMU_BATTERY_STATE, 0, NULL, buf);
	if (len != 9)
		return -1;	

	sc->sc_flags = buf[1];

	if (out) {
		if (buf[1] & PMU_PWR_AC_PRESENT)
			printf(" AC");
		if (buf[1] & PMU_PWR_BATT_CHARGING)
			printf(" charging");
		if (buf[1] & PMU_PWR_BATT_PRESENT)
			printf(" present");
		if (buf[1] & PMU_PWR_BATT_FULL)
			printf(" full");
		printf("\n");
	}

	sc->sc_cpu_temp = buf[4];

	if ((sc->sc_flags & PMU_PWR_BATT_PRESENT) == 0) {
		sc->sc_voltage = 0;
		sc->sc_current = 0;
		sc->sc_bat_temp = 0;
		sc->sc_charge = 0;
		sc->sc_time = 0;
		return 0;
	}

	vmax = sc->sc_vmax_charged;
	vb = (buf[2] << 8) | buf[3];
	sc->sc_voltage = (vb * 265 + 72665) / 10;
	sc->sc_current = buf[6];
	if ((sc->sc_flags & PMU_PWR_AC_PRESENT) == 0) { 
	    	if (sc->sc_current > 200)
			vb += ((sc->sc_current - 200) * 15) / 100;
	} else {
		vmax = sc->sc_vmax_charging;
	}
	sc->sc_charge = (100 * vb) / vmax;
	if (sc->sc_flags & PMU_PWR_PCHARGE_RESET) {
		pcharge = (buf[7] << 8) | buf[8];
		if (pcharge > 6500)
			pcharge = 6500;
		pcharge = 100 - pcharge * 100 / 6500;
		if (pcharge < sc->sc_charge)
			sc->sc_charge = pcharge;
	}
	if (sc->sc_current > 0) {
		sc->sc_time = (sc->sc_charge * 16440) / sc->sc_current;
	} else 
		sc->sc_time = 0;
	
	sc->sc_bat_temp = buf[5];

	if (out) {
		printf("voltage: %d.%03d\n", sc->sc_voltage / 1000,
		    sc->sc_voltage % 1000);
		printf("charge:  %d%%\n", sc->sc_charge);
		if (sc->sc_time > 0)
			printf("time:    %d:%02d\n", sc->sc_time / 60, 
			    sc->sc_time % 60);
	}

	return 0;
}

#define INITDATA(index, unit, string) \
	sc->sc_sdata[index].sensor = index;				\
	sc->sc_sdata[index].units = unit;     				\
	sc->sc_sdata[index].validflags = ENVSYS_FVALID;			\
	sc->sc_sdata[index].warnflags = 0;				\
	sc->sc_sinfo[index].sensor = index;				\
	sc->sc_sinfo[index].units = unit;     				\
	sc->sc_sinfo[index].rfact = 1;     				\
	sc->sc_sinfo[index].validflags = ENVSYS_FVALID;			\
	snprintf(sc->sc_sinfo[index].desc, sizeof(sc->sc_sinfo[index].desc),	\
	    "%s", string);

static void
battery_setup_envsys(struct battery_softc *sc)
{

	INITDATA(BAT_CPU_TEMPERATURE, ENVSYS_STEMP, "CPU temperature");
	INITDATA(BAT_AC_PRESENT, ENVSYS_INDICATOR, "AC present");
	INITDATA(BAT_PRESENT, ENVSYS_INDICATOR, "Battery present");
	INITDATA(BAT_VOLTAGE, ENVSYS_SVOLTS_DC, "Battery voltage");
	INITDATA(BAT_CHARGE, ENVSYS_INTEGER, "Battery charge");
	INITDATA(BAT_MAX_CHARGE, ENVSYS_INTEGER, "Battery design cap");
	INITDATA(BAT_CURRENT, ENVSYS_SAMPS, "Battery current");
	INITDATA(BAT_TEMPERATURE, ENVSYS_STEMP, "Battery temperature");
	INITDATA(BAT_CHARGING, ENVSYS_INDICATOR, "Battery charging");
	INITDATA(BAT_DISCHARGING, ENVSYS_INDICATOR, "Battery discharging");
	INITDATA(BAT_FULL, ENVSYS_INDICATOR, "Battery full");
#undef INITDATA

	sc->sc_sysmon.sme_sensor_info = sc->sc_sinfo;
	sc->sc_sysmon.sme_sensor_data = sc->sc_sdata;
	sc->sc_sysmon.sme_cookie = sc;
	sc->sc_sysmon.sme_gtredata = battery_gtredata;
	sc->sc_sysmon.sme_streinfo = battery_streinfo;
	sc->sc_sysmon.sme_nsensors = BAT_NSENSORS;
	sc->sc_sysmon.sme_envsys_version = 1000;

	if (sysmon_envsys_register(&sc->sc_sysmon))
		printf("%s: unable to register with sysmon\n",
		    sc->sc_dev.dv_xname);
}

static int
battery_gtredata(struct sysmon_envsys *sme, struct envsys_tre_data *tred)
{
	struct battery_softc *sc = sme->sme_cookie;
	struct envsys_tre_data *cur;
	int which = tred->sensor;

	battery_update(sc, 0);

	cur = &sc->sc_sdata[BAT_CPU_TEMPERATURE];
	cur->cur.data_s = sc->sc_cpu_temp * 1000000 + 273150000;
	cur->validflags = ENVSYS_FVALID | ENVSYS_FCURVALID;

	cur = &sc->sc_sdata[BAT_AC_PRESENT];
	cur->cur.data_s = (sc->sc_flags & PMU_PWR_AC_PRESENT);
	cur->validflags = ENVSYS_FVALID | ENVSYS_FCURVALID;

	cur = &sc->sc_sdata[BAT_PRESENT];
	cur->cur.data_s = (sc->sc_flags & PMU_PWR_BATT_PRESENT);
	cur->validflags = ENVSYS_FVALID | ENVSYS_FCURVALID;

	cur = &sc->sc_sdata[BAT_VOLTAGE];
	cur->cur.data_s = sc->sc_voltage * 1000;
	cur->validflags = ENVSYS_FVALID | ENVSYS_FCURVALID;

	cur = &sc->sc_sdata[BAT_CURRENT];
	cur->cur.data_s = sc->sc_current * 1000;
	cur->validflags = ENVSYS_FVALID | ENVSYS_FCURVALID;

	cur = &sc->sc_sdata[BAT_CHARGE];
	cur->cur.data_s = sc->sc_charge;
	cur->validflags = ENVSYS_FVALID | ENVSYS_FCURVALID;

	cur = &sc->sc_sdata[BAT_MAX_CHARGE];
	cur->cur.data_s = 100;
	cur->validflags = ENVSYS_FVALID | ENVSYS_FCURVALID;

	cur = &sc->sc_sdata[BAT_TEMPERATURE];
	cur->cur.data_s = sc->sc_bat_temp * 1000000 + 273150000;
	cur->validflags = ENVSYS_FVALID | ENVSYS_FCURVALID;

	cur = &sc->sc_sdata[BAT_CHARGING];
	cur->cur.data_s = (sc->sc_flags & PMU_PWR_BATT_CHARGING);
	cur->validflags = ENVSYS_FVALID | ENVSYS_FCURVALID;

	cur = &sc->sc_sdata[BAT_DISCHARGING];
	cur->cur.data_s = (!(sc->sc_flags & PMU_PWR_BATT_CHARGING)) && 
	    (!(sc->sc_flags & PMU_PWR_AC_PRESENT));
	cur->validflags = ENVSYS_FVALID | ENVSYS_FCURVALID;

	cur = &sc->sc_sdata[BAT_FULL];
	cur->cur.data_s = (sc->sc_flags & PMU_PWR_BATT_FULL);
	cur->validflags = ENVSYS_FVALID | ENVSYS_FCURVALID;

	memcpy(tred, &sc->sc_sdata[which], sizeof(struct envsys_tre_data));
	return 0;
}

static int
battery_streinfo(struct sysmon_envsys *sme, struct envsys_basic_info *binfo)
{

	/* XXX Not implemented */
	binfo->validflags = 0;

	return 0;
}
