/*	$NetBSD: linux32_exec.c,v 1.1 2006/02/09 19:18:57 manu Exp $ */

/*-
 * Copyright (c) 1994-2006 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Christos Zoulas, Frank van der Linden, Eric Haszlakiewicz,
 * Thor Lancelot Simon, and Emmanuel Dreyfus.
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

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: linux32_exec.c,v 1.1 2006/02/09 19:18:57 manu Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/malloc.h>
#include <sys/namei.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/exec.h>
#include <sys/exec_elf.h>

#include <sys/mman.h>
#include <sys/sa.h>
#include <sys/syscallargs.h>

#include <uvm/uvm_extern.h>

#include <machine/cpu.h>
#include <machine/reg.h>

#include <compat/linux/common/linux_types.h>

#include <compat/linux32/common/linux32_exec.h>
#include <compat/linux32/common/linux32_types.h>
#include <compat/linux32/common/linux32_signal.h>
#include <compat/linux32/common/linux32_machdep.h>
#include <compat/linux32/common/linux32_emuldata.h>

#include <compat/linux32/linux32_syscallargs.h>
#include <compat/linux32/linux32_syscall.h>

extern char linux32_sigcode[1];
extern char linux32_rt_sigcode[1];
extern char linux32_esigcode[1];

extern struct sysent linux32_sysent[];
extern const char * const linux32_syscallnames[];

static void linux32_e_proc_exec __P((struct proc *, struct exec_package *));
static void linux32_e_proc_fork __P((struct proc *, struct proc *, int));
static void linux32_e_proc_exit __P((struct proc *));
static void linux32_e_proc_init __P((struct proc *, struct proc *, int));

#ifdef LINUX32_NPTL
static void linux32_userret __P((struct lwp *, void *));
#endif

/*
 * Emulation switch.
 */

struct uvm_object *emul_linux32_object;

const struct emul emul_linux32 = {
	"linux32",
	"/emul/linux32",
#ifndef __HAVE_MINIMAL_EMUL
	0,
	NULL,
	LINUX32_SYS_syscall,
	LINUX32_SYS_NSYSENT,
#endif
	linux32_sysent,
	linux32_syscallnames,
	linux32_sendsig,
	trapsignal,
	NULL,
	linux32_sigcode,
	linux32_esigcode,
	&emul_linux32_object,
	linux32_setregs,
	linux32_e_proc_exec,
	linux32_e_proc_fork,
	linux32_e_proc_exit,
	NULL,
	NULL,
	linux32_syscall_intern,
	NULL,
	NULL,
	netbsd32_vm_default_addr,
};

static void
linux32_e_proc_init(p, parent, forkflags)
	struct proc *p, *parent;
	int forkflags;
{
	struct linux32_emuldata *e = p->p_emuldata;
	struct linux32_emuldata_shared *s;
	struct linux32_emuldata *ep = NULL;

	if (!e) {
		/* allocate new Linux emuldata */
		MALLOC(e, void *, sizeof(struct linux32_emuldata),
			M_EMULDATA, M_WAITOK);
	} else  {
		e->s->refs--;
		if (e->s->refs == 0)
			FREE(e->s, M_EMULDATA);
	}

	memset(e, '\0', sizeof(struct linux32_emuldata));

	if (parent)
		ep = parent->p_emuldata;

	if (forkflags & FORK_SHAREVM) {
#ifdef DIAGNOSTIC
		if (ep == NULL) {
			killproc(p, "FORK_SHAREVM while emuldata is NULL\n");
			return;
		}
#endif
		s = ep->s;
		s->refs++;
	} else {
		struct vmspace *vm;

		MALLOC(s, void *, sizeof(struct linux32_emuldata_shared),
			M_EMULDATA, M_WAITOK);
		s->refs = 1;

		/*
		 * Set the process idea of the break to the real value.
		 * For fork, we use parent's vmspace since our's
		 * is not setup at the time of this call and is going
		 * to be copy of parent's anyway. For exec, just
		 * use our own vmspace.
		 */
		vm = (parent) ? parent->p_vmspace : p->p_vmspace;
		s->p_break = vm->vm_daddr + ctob(vm->vm_dsize);

		/*
		 * Linux threads are emulated as NetBSD processes (not lwp)
		 * We use native PID for Linux TID. The Linux TID is the
		 * PID of the first process in the group. It is stored
		 * here
		 */
		s->group_pid = p->p_pid;
	}

	e->s = s;

#ifdef LINUX32_NPTL
	/* 
	 * initialize TID pointers. ep->child_clear_tid and 
	 * ep->child_set_tid will not be used beyond this point.
	 */
	e->child_clear_tid = NULL;
	e->child_set_tid = NULL;
	if (ep != NULL) {
		e->clear_tid = ep->child_clear_tid;
		e->set_tid = ep->child_set_tid;
		e->set_tls = ep->set_tls;
		ep->child_clear_tid = NULL;
		ep->child_set_tid = NULL;
		ep->set_tls = 0;
	} else {
		e->clear_tid = NULL;
		e->set_tid = NULL;
		e->set_tls = 0;
	}
#endif /* LINUX32_NPTL */

	p->p_emuldata = e;
}

/*
 * Allocate new per-process structures. Called when executing Linux
 * process. We can reuse the old emuldata - if it's not null,
 * the executed process is of same emulation as original forked one.
 */
static void
linux32_e_proc_exec(p, epp)
	struct proc *p;
	struct exec_package *epp;
{
	/* exec, use our vmspace */
	linux32_e_proc_init(p, NULL, 0);
}

/*
 * Emulation per-process exit hook.
 */
static void
linux32_e_proc_exit(p)
	struct proc *p;
{
	struct linux32_emuldata *e = p->p_emuldata;

#ifdef LINUX32_NPTL
	/* Emulate LINUX_CLONE_CHILD_CLEARTID */
	if (e->clear_tid != NULL) {
		int error;
		int null = 0;
		struct linux32_sys_futex_args cup;
		register_t retval;
		struct lwp *l;

		if ((error = copyout(&null, 
		    e->clear_tid, 
		    sizeof(null))) != 0)
			printf("linux32_e_proc_exit: cannot clear TID\n");

		l = proc_representative_lwp(p);
		SCARG(&cup, uaddr) = e->clear_tid;
		SCARG(&cup, op) = LINUX_FUTEX_WAKE;
		SCARG(&cup, val) = 0x7fffffff; /* Awake everyone */
		SCARG(&cup, timeout) = NULL;
		SCARG(&cup, uaddr2) = NULL;
		SCARG(&cup, val3) = 0;
		if ((error = linux32_sys_futex(l, &cup, &retval)) != 0)
			printf("linux32_e_proc_exit: linux_sys_futex failed\n");
	}
#endif /* LINUX32_NPTL */

	/* free Linux emuldata and set the pointer to null */
	e->s->refs--;
	if (e->s->refs == 0)
		FREE(e->s, M_EMULDATA);
	FREE(e, M_EMULDATA);
	p->p_emuldata = NULL;
}

/*
 * Emulation fork hook.
 */
static void
linux32_e_proc_fork(p, parent, forkflags)
	struct proc *p, *parent;
	int forkflags;
{
#ifdef LINUX32_NPTL
	struct linux32_emuldata *e;
#endif

	/*
	 * The new process might share some vmspace-related stuff
	 * with parent, depending on fork flags (CLONE_VM et.al).
	 * Force allocation of new base emuldata, and share the
	 * VM-related parts only if necessary.
	 */
	p->p_emuldata = NULL;
	linux32_e_proc_init(p, parent, forkflags);

#ifdef LINUX32_NPTL
	/* 
	 * Emulate LINUX_CLONE_CHILD_SETTID and LINUX_CLONE_TLS:
	 * This cannot be done right now because the child VM 
	 * is not set up. We will do it at userret time.
	 */
	e = p->p_emuldata;
	if ((e->set_tid != NULL) || (e->set_tls != 0))
		p->p_userret = (*linux32_userret);
#endif

	return;
}

#ifdef LINUX32_NPTL
static void
linux32_userret(l, arg)
	struct lwp *l;
	void *arg;
{
	struct proc *p = l->l_proc;
	struct linux32_emuldata *led = p->p_emuldata;
	int error;

	p->p_userret = NULL;

	/* Emulate LINUX_CLONE_CHILD_SETTID  */
	if (led->set_tid != NULL) {
		if ((error = copyout(&p->p_pid,
		    led->set_tid, sizeof(p->p_pid))) != 0)
			printf("linux32_userret: cannot set TID\n");
	}

	/* Emulate LINUX_CLONE_NEWTLS */
	if (led->set_tls != 0) {
		if (linux32_set_newtls(l, led->set_tls) != 0)
			printf("linux32_userret: cannot set TLS\n");
	}

	return;	
}
#endif /* LINUX32_NPTL */
