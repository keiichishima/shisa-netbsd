/*      $NetBSD: clockctl.c,v 1.13 2005/02/27 00:26:58 perry Exp $ */

/*-
 * Copyright (c) 2001 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Emmanuel Dreyfus.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
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

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: clockctl.c,v 1.13 2005/02/27 00:26:58 perry Exp $");

#include "opt_ntp.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/device.h>
#include <sys/time.h>
#include <sys/conf.h>
#ifdef NTP
#include <sys/timex.h>
#endif /* NTP */

#include <sys/clockctl.h>

struct clockctl_softc {
	struct device   clockctl_dev;
};

dev_type_ioctl(clockctlioctl);

const struct cdevsw clockctl_cdevsw = {
	nullopen, nullclose, noread, nowrite, clockctlioctl,
	nostop, notty, nopoll, nommap, nokqfilter,
};

/*ARGSUSED*/
void
clockctlattach(int num)
{
	/* Nothing to set up before open is called */
	return;
}

int
clockctlioctl(dev, cmd, data, flags, p)
	dev_t dev;
	u_long cmd;
	caddr_t data;
	int flags;
	struct proc *p;
{
	int error = 0;

	switch (cmd) {
		case CLOCKCTL_SETTIMEOFDAY: {
			struct sys_settimeofday_args *args =
			    (struct sys_settimeofday_args *)data;

			error = settimeofday1(SCARG(args, tv),
			    SCARG(args, tzp), p);
			if (error)
				return (error);
			break;
		}
		case CLOCKCTL_ADJTIME: {
			struct sys_adjtime_args *args =
			    (struct sys_adjtime_args *)data;

			error = adjtime1(SCARG(args, delta),
			    SCARG(args, olddelta), p);
			if (error)
				return (error);
			break;
		}
		case CLOCKCTL_CLOCK_SETTIME: {
			struct sys_clock_settime_args *args =
			    (struct sys_clock_settime_args *)data;

			error = clock_settime1(SCARG(args, clock_id),
			    SCARG(args, tp));
			if (error)
				return (error);
			break;
		}
#ifdef NTP
		case CLOCKCTL_NTP_ADJTIME: {
			struct clockctl_ntp_adjtime_args *args =
			    (struct clockctl_ntp_adjtime_args *)data;
			struct timex ntv;

			error = copyin((caddr_t)SCARG(args,uas.tp),
			    (caddr_t)&ntv, sizeof(ntv));
			if (error)
				return (error);

			error = ntp_adjtime1(&ntv, args, &args->retval);
			return (error);
		}
#endif /* NTP */
		default:
			error = EINVAL;
	}

	return (error);
}


