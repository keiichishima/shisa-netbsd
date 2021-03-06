/*	$NetBSD: linux32_stat.c,v 1.1 2006/02/09 19:18:57 manu Exp $ */

/*-
 * Copyright (c) 2006 Emmanuel Dreyfus, all rights reserved.
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
 *	This product includes software developed by Emmanuel Dreyfus
 * 4. The name of the author may not be used to endorse or promote 
 *    products derived from this software without specific prior written 
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE THE AUTHOR AND CONTRIBUTORS ``AS IS'' 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS 
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>

__KERNEL_RCSID(0, "$NetBSD: linux32_stat.c,v 1.1 2006/02/09 19:18:57 manu Exp $");

#include <sys/types.h>
#include <sys/param.h>
#include <sys/fstypes.h>
#include <sys/signal.h>
#include <sys/dirent.h>
#include <sys/kernel.h>
#include <sys/fcntl.h>
#include <sys/select.h>
#include <sys/sa.h>
#include <sys/proc.h>
#include <sys/ucred.h>
#include <sys/swap.h>

#include <machine/types.h>

#include <sys/syscallargs.h>

#include <compat/netbsd32/netbsd32.h>
#include <compat/netbsd32/netbsd32_conv.h>
#include <compat/netbsd32/netbsd32_syscallargs.h>

#include <compat/linux/common/linux_types.h>
#include <compat/linux/common/linux_signal.h>
#include <compat/linux/common/linux_machdep.h>
#include <compat/linux/common/linux_misc.h>
#include <compat/linux/common/linux_oldolduname.h>
#include <compat/linux/linux_syscallargs.h>

#include <compat/linux32/common/linux32_types.h>
#include <compat/linux32/common/linux32_signal.h>
#include <compat/linux32/common/linux32_machdep.h>
#include <compat/linux32/common/linux32_sysctl.h>
#include <compat/linux32/common/linux32_socketcall.h>
#include <compat/linux32/linux32_syscallargs.h>

static inline void linux32_from_stat(struct stat *, struct linux32_stat64 *);

#define linux_fakedev(x,y) (x)
static inline void
linux32_from_stat(st, st32)
	struct stat *st;
	struct linux32_stat64 *st32;
{
	bzero(st32, sizeof(*st32));
	st32->lst_dev = linux_fakedev(st->st_dev, 0);
	st32->__lst_ino = st->st_ino;
	st32->lst_mode = st->st_mode;
	if (st->st_nlink >= (1 << 15))
		st32->lst_nlink = (1 << 15) - 1;
	else
		st32->lst_nlink = st->st_nlink;
	st32->lst_uid = st->st_uid;
	st32->lst_gid = st->st_gid;
	st32->lst_rdev = linux_fakedev(st->st_rdev, 0);
	st32->lst_size = st->st_size;
	st32->lst_blksize = st->st_blksize;
	st32->lst_blocks = st->st_blocks;
	st32->lst_atime = st->st_atime;
	st32->lst_mtime = st->st_mtime;
	st32->lst_ctime = st->st_ctime;
#ifdef LINUX32_STAT64_HAS_NSEC
	st32->lst_atime_nsec = st->st_atimensec;
	st32->lst_mtime_nsec = st->st_mtimensec;
	st32->lst_ctime_nsec = st->st_ctimensec;
#endif
#ifdef LINUX32_STAT64_HAS_BROKEN_ST_INO
	st32->lst_ino = st->st_ino;
#endif

	return;
}

int
linux32_sys_stat64(l, v, retval)  
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct linux32_sys_stat64_args /* {  
	        syscallarg(netbsd32_charp) path;
	        syscallarg(linux32_statp) sp;
	} */ *uap = v;
	struct sys___stat30_args ua;
	caddr_t sg = stackgap_init(l->l_proc, 0);
	int error;
	struct stat st;
	struct linux32_stat64 st32;
	struct stat *stp;
	struct linux32_stat64 *st32p;
	
	NETBSD32TOP_UAP(path, const char);

	CHECK_ALT_EXIST(l, &sg, SCARG(&ua, path));

	stp = stackgap_alloc(l->l_proc, &sg, sizeof(*stp));
	SCARG(&ua, ub) = stp;
		
	if ((error = sys___stat30(l, &ua, retval)) != 0)
		return error;

	if ((error = copyin(stp, &st, sizeof(st))) != 0)
		return error;
	
	linux32_from_stat(&st, &st32);

	st32p = (struct linux32_stat64 *)(long)SCARG(uap, sp);

	if ((error = copyout(&st32, st32p, sizeof(*st32p))) != 0)
		return error;

	return 0;
}

int
linux32_sys_lstat64(l, v, retval)  
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct linux32_sys_lstat64_args /* {  
	        syscallarg(netbsd32_charp) path;
	        syscallarg(linux32_stat64p) sp;
	} */ *uap = v;
	struct sys___lstat30_args ua;
	caddr_t sg = stackgap_init(l->l_proc, 0);
	int error;
	struct stat st;
	struct linux32_stat64 st32;
	struct stat *stp;
	struct linux32_stat64 *st32p;
	
	NETBSD32TOP_UAP(path, const char);

	CHECK_ALT_EXIST(l, &sg, SCARG(&ua, path));

	stp = stackgap_alloc(l->l_proc, &sg, sizeof(*stp));
	SCARG(&ua, ub) = stp;
		
	if ((error = sys___lstat30(l, &ua, retval)) != 0)
		return error;

	if ((error = copyin(stp, &st, sizeof(st))) != 0)
		return error;
	
	linux32_from_stat(&st, &st32);

	st32p = (struct linux32_stat64 *)(long)SCARG(uap, sp);

	if ((error = copyout(&st32, st32p, sizeof(*st32p))) != 0)
		return error;

	return 0;
}

int
linux32_sys_fstat64(l, v, retval)  
	struct lwp *l;
	void *v;
	register_t *retval;
{
	struct linux32_sys_fstat64_args /* {  
	        syscallarg(int) fd;
	        syscallarg(linux32_stat64p) sp;
	} */ *uap = v;
	struct sys___fstat30_args ua;
	caddr_t sg = stackgap_init(l->l_proc, 0);
	int error;
	struct stat st;
	struct linux32_stat64 st32;
	struct stat *stp;
	struct linux32_stat64 *st32p;
	
	NETBSD32TO64_UAP(fd);

	stp = stackgap_alloc(l->l_proc, &sg, sizeof(*stp));
	SCARG(&ua, sb) = stp;
		
	if ((error = sys___fstat30(l, &ua, retval)) != 0)
		return error;

	if ((error = copyin(stp, &st, sizeof(st))) != 0)
		return error;
	
	linux32_from_stat(&st, &st32);

	st32p = (struct linux32_stat64 *)(long)SCARG(uap, sp);

	if ((error = copyout(&st32, st32p, sizeof(*st32p))) != 0)
		return error;

	return 0;
}
