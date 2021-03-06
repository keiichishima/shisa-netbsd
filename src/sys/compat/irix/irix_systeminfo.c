/*	$NetBSD: irix_systeminfo.c,v 1.10 2005/02/26 23:10:19 perry Exp $ */

/*-
 * Copyright (c) 2002 The NetBSD Foundation, Inc.
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
__KERNEL_RCSID(0, "$NetBSD: irix_systeminfo.c,v 1.10 2005/02/26 23:10:19 perry Exp $");

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/systm.h>
#include <sys/sysctl.h>
#include <sys/sa.h>
#include <sys/syscallargs.h>

#include <compat/common/compat_util.h>

#include <compat/svr4/svr4_systeminfo.h>
#include <compat/svr4/svr4_types.h>
#include <compat/svr4/svr4_signal.h>
#include <compat/svr4/svr4_ucontext.h>
#include <compat/svr4/svr4_lwp.h>
#include <compat/svr4/svr4_syscallargs.h>

#include <compat/irix/irix_types.h>
#include <compat/irix/irix_signal.h>
#include <compat/irix/irix_sysctl.h>
#include <compat/irix/irix_syscallargs.h>

char irix_si_vendor[128] = "Silicon Graphics, Inc.";
char irix_si_os_provider[128] = "Silicon Graphics, Inc.";
char irix_si_os_name[128] = "IRIX";
char irix_si_hw_name[128] = "IP22"; /* XXX */
char irix_si_osrel_maj[128] = "6";
char irix_si_osrel_min[128] = "5";
char irix_si_osrel_patch[128] = "0";
char irix_si_processors[128] = "R5000 1.0"; /* XXX */
char irix_si_version[128] = "04131232";

#define BUF_SIZE 16

int
irix_sys_systeminfo(l, v, retval)
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct irix_sys_systeminfo_args /* {
		syscallarg(int) what;
		syscallarg(char *) buf;
		syscallarg(long) len;
	} */ *uap = v;
	const char *str = NULL;
	char strbuf[BUF_SIZE + 1];
	int error = 0;
	size_t len = 0;
	char buf[256];

	u_int rlen = SCARG(uap, len);

	switch (SCARG(uap, what)) {
	case SVR4_MIPS_SI_VENDOR:
		str = irix_si_vendor;
		break;

	case SVR4_MIPS_SI_OS_PROVIDER:
		str = irix_si_os_provider;
		break;

	case SVR4_MIPS_SI_OS_NAME:
		str = irix_si_os_name;
		break;

	case SVR4_MIPS_SI_HW_NAME:
		str = irix_si_hw_name;
		break;

	case SVR4_MIPS_SI_OSREL_MAJ:
		str = irix_si_osrel_maj;
		break;

	case SVR4_MIPS_SI_OSREL_MIN:
		str = irix_si_osrel_min;
		break;

	case SVR4_MIPS_SI_OSREL_PATCH:
		str = irix_si_osrel_patch;
		break;

	case SVR4_MIPS_SI_PROCESSORS:
		str = irix_si_processors;
		break;

	case SVR4_MIPS_SI_NUM_PROCESSORS:
	case SVR4_MIPS_SI_AVAIL_PROCESSORS: {
		int ncpu, name[2];
		size_t sz;

		name[0] = CTL_HW;
		name[1] = HW_NCPU;
		sz = sizeof(ncpu);
		error = old_sysctl(&name[0], 2, &ncpu, &sz, NULL, 0, NULL);
		if (error)
			return error;
		snprintf(strbuf, BUF_SIZE, "%d", ncpu);
		str = strbuf;

		break;
	}

	case SVR4_SI_HW_SERIAL:
		snprintf(strbuf, BUF_SIZE, "%d", (int32_t)hostid);
		str = strbuf;
		break;

	case SVR4_MIPS_SI_HOSTID:
		snprintf(strbuf, BUF_SIZE, "%08x", (int32_t)hostid);
		str = strbuf;
		break;

	case SVR4_MIPS_SI_SERIAL: /* Unimplemented yet */
	default:
		return svr4_sys_systeminfo(l, v, retval);
		break;
	}

	/*
	 * This duplicates some code in
	 * svr4_sys_systeminfo().
	 * Ideally, it should be merged.
	 */
	if (str) {
		len = strlen(str) + 1;
		if (len > rlen)
			len = rlen;

		if (SCARG(uap, buf)) {
			error = copyout(str, SCARG(uap, buf), len);
			if (error)
				return error;
			/* make sure we are NULL terminated */
			buf[0] = '\0';
			error = copyout(buf, &(SCARG(uap, buf)[len - 1]), 1);
		}
		else
			error = 0;
	}

	*retval = len;
	return error;
}
