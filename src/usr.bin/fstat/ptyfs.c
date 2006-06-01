/*	$NetBSD: ptyfs.c,v 1.2 2004/12/14 03:10:23 atatat Exp $	*/

/*-
 * Copyright (c) 2004 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Christos Zoulas.
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
__RCSID("$NetBSD: ptyfs.c,v 1.2 2004/12/14 03:10:23 atatat Exp $");

#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/stat.h>
#include <sys/vnode.h>
#include <sys/mount.h>

#define _KERNEL
#include <miscfs/specfs/specdev.h>
#include <fs/ptyfs/ptyfs.h>
#undef _KERNEL

#include <paths.h>
#include <err.h>
#include <kvm.h>
#include <dirent.h>
#include "fstat.h"

int
ptyfs_filestat(struct vnode *vp, struct filestat *fsp)
{
	struct ptyfsnode pn;
	struct specinfo si;
	struct mount mt;

	if (!KVM_READ(VTOPTYFS(vp), &pn, sizeof(pn))) {
		dprintf("can't read ptyfs_node at %p for pid %d",
		    VTOPTYFS(vp), Pid);
		return 0;
	}
	if (!KVM_READ(vp->v_mount, &mt, sizeof(mt))) {
		dprintf("can't read mount at %p for pid %d",
		    VTOPTYFS(vp), Pid);
		return 0;
	}
	fsp->fsid = mt.mnt_stat.f_fsidx.__fsid_val[0];
	fsp->fileid = (long)pn.ptyfs_fileno;
	fsp->mode = pn.ptyfs_mode;
	fsp->size = 0;
	switch (pn.ptyfs_type) {
	    case PTYFSpts:
	    case PTYFSptc:
		if (!KVM_READ(vp->v_specinfo, &si, sizeof(si))) {
			dprintf("can't read specinfo at %p for pid %d",
				VTOPTYFS(vp), Pid);
			return 0;
		}
		fsp->rdev = si.si_rdev;
		fsp->mode |= S_IFCHR;
		break;
	    case PTYFSroot:
		fsp->rdev = 0;
		fsp->mode |= S_IFDIR;
		break;
	}

	return 1;
}
