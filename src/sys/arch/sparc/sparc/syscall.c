/*	$NetBSD: syscall.c,v 1.9 2006/03/07 07:21:51 thorpej Exp $ */

/*
 * Copyright (c) 1996
 *	The President and Fellows of Harvard College. All rights reserved.
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This software was developed by the Computer Systems Engineering group
 * at Lawrence Berkeley Laboratory under DARPA contract BG 91-66 and
 * contributed to Berkeley.
 *
 * All advertising materials mentioning features or use of this software
 * must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Lawrence Berkeley Laboratory.
 *	This product includes software developed by Harvard University.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 *	This product includes software developed by Harvard University.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)trap.c	8.4 (Berkeley) 9/23/93
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: syscall.c,v 1.9 2006/03/07 07:21:51 thorpej Exp $");

#include "opt_ktrace.h"
#include "opt_sparc_arch.h"
#include "opt_multiprocessor.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/signal.h>
#include <sys/sa.h>
#include <sys/savar.h>
#include <sys/syscall.h>
#ifdef KTRACE
#include <sys/ktrace.h>
#endif

#include <uvm/uvm_extern.h>

#include <sparc/sparc/asm.h>
#include <machine/cpu.h>
#include <machine/ctlreg.h>
#include <machine/trap.h>
#include <machine/instr.h>
#include <machine/pmap.h>
#include <machine/userret.h>

#ifdef DDB
#include <machine/db_machdep.h>
#else
#include <machine/frame.h>
#endif

#include <sparc/fpu/fpu_extern.h>
#include <sparc/sparc/memreg.h>
#include <sparc/sparc/cpuvar.h>

#define MAXARGS	8
struct args {
	register_t i[MAXARGS];
};

static inline int handle_new(struct trapframe *, register_t *);
static inline int getargs(struct proc *p, struct trapframe *,
    register_t *, const struct sysent **, struct args *);
#ifdef FPU_DEBUG
static inline void save_fpu(struct trapframe *);
#endif
void syscall_plain(register_t, struct trapframe *, register_t);
void syscall_fancy(register_t, struct trapframe *, register_t);

static inline int
handle_new(struct trapframe *tf, register_t *code)
{
	int new = *code & (SYSCALL_G7RFLAG | SYSCALL_G2RFLAG);
	*code &= ~(SYSCALL_G7RFLAG | SYSCALL_G2RFLAG);
	return new;
}

/*
 * The first six system call arguments are in the six %o registers.
 * Any arguments beyond that are in the `argument extension' area
 * of the user's stack frame (see <machine/frame.h>).
 *
 * Check for ``special'' codes that alter this, namely syscall and
 * __syscall.  The latter takes a quad syscall number, so that other
 * arguments are at their natural alignments.  Adjust the number
 * of ``easy'' arguments as appropriate; we will copy the hard
 * ones later as needed.
 */
static inline int
getargs(struct proc *p, struct trapframe *tf, register_t *code,
    const struct sysent **callp, struct args *args)
{
	int *ap = &tf->tf_out[0];
	int error, i, nap = 6;

	*callp = p->p_emul->e_sysent;

	switch (*code) {
	case SYS_syscall:
		*code = *ap++;
		nap--;
		break;
	case SYS___syscall:
		if (!(p->p_emul->e_flags & EMUL_HAS_SYS___syscall))
			break;
		*code = ap[_QUAD_LOWWORD];
		ap += 2;
		nap -= 2;
		break;
	}

	if (*code < 0 || *code >= p->p_emul->e_nsysent)
		return ENOSYS;

	*callp += *code;
	i = (*callp)->sy_argsize / sizeof(register_t);
	if (__predict_false(i > nap)) {	/* usually false */
		void *off = (char *)tf->tf_out[6] +
		    offsetof(struct frame, fr_argx);
#ifdef DIAGNOSTIC
		KASSERT(i <= MAXARGS);
#endif
		error = copyin(off, &args->i[nap], (i - nap) * sizeof(*ap));
		if (error)
			return error;
		i = nap;
	}
	copywords(ap, args->i, i * sizeof(*ap));
	return 0;
}

#ifdef FPU_DEBUG
static inline void
save_fpu(struct trapframe *tf)
{
	struct lwp *l = curlwp;

	if ((tf->tf_psr & PSR_EF) != 0) {
		if (cpuinfo.fplwp != l)
			panic("FPU enabled but wrong proc (3) [l=%p, fwlp=%p]",
				l, cpuinfo.fplwp);
		savefpstate(l->l_md.md_fpstate);
		l->l_md.md_fpu = NULL;
		cpuinfo.fplwp = NULL;
		tf->tf_psr &= ~PSR_EF;
		setpsr(getpsr() & ~PSR_EF);
	}
}
#endif

void
syscall_intern(struct proc *p)
{

	if (trace_is_enabled(p))
		p->p_md.md_syscall = syscall_fancy;
	else
		p->p_md.md_syscall = syscall_plain;
}

/*
 * System calls.  `pc' is just a copy of tf->tf_pc.
 *
 * Note that the things labelled `out' registers in the trapframe were the
 * `in' registers within the syscall trap code (because of the automatic
 * `save' effect of each trap).  They are, however, the %o registers of the
 * thing that made the system call, and are named that way here.
 */
void
syscall_plain(register_t code, struct trapframe *tf, register_t pc)
{
	const struct sysent *callp;
	struct proc *p;
	struct lwp *l;
	int error, new;
	struct args args;
	register_t rval[2], i;
	u_quad_t sticks;

	uvmexp.syscalls++;	/* XXXSMP */
	l = curlwp;
	p = l->l_proc;

	sticks = p->p_sticks;
	l->l_md.md_tf = tf;

#ifdef FPU_DEBUG
	save_fpu(tf);
#endif
	new = handle_new(tf, &code);

	if ((error = getargs(p, tf, &code, &callp, &args)) != 0)
		goto bad;

	rval[0] = 0;
	rval[1] = tf->tf_out[1];

        /* Lock the kernel if the syscall isn't MP-safe. */
	if (callp->sy_flags & SYCALL_MPSAFE) {
		error = (*callp->sy_call)(l, &args, rval);
	} else {
		KERNEL_PROC_LOCK(l);
		error = (*callp->sy_call)(l, &args, rval);
		KERNEL_PROC_UNLOCK(l);
	}

	switch (error) {
	case 0:
		/* Note: fork() does not return here in the child */
		tf->tf_out[0] = rval[0];
		tf->tf_out[1] = rval[1];
		if (new) {
			/* jmp %g2 (or %g7, deprecated) on success */
			i = tf->tf_global[new & SYSCALL_G2RFLAG ? 2 : 7];
			if (i & 3) {
				error = EINVAL;
				goto bad;
			}
		} else {
			/* old system call convention: clear C on success */
			tf->tf_psr &= ~PSR_C;	/* success */
			i = tf->tf_npc;
		}
		tf->tf_pc = i;
		tf->tf_npc = i + 4;
		break;

	case ERESTART:
	case EJUSTRETURN:
		/* nothing to do */
		break;

	default:
	bad:
		if (p->p_emul->e_errno)
			error = p->p_emul->e_errno[error];
		tf->tf_out[0] = error;
		tf->tf_psr |= PSR_C;	/* fail */
		i = tf->tf_npc;
		tf->tf_pc = i;
		tf->tf_npc = i + 4;
		break;
	}

	userret(l, pc, sticks);
	share_fpu(l, tf);
}

void
syscall_fancy(register_t code, struct trapframe *tf, register_t pc)
{
	const struct sysent *callp;
	struct proc *p;
	struct lwp *l;
	int error, new;
	struct args args;
	register_t rval[2], i;
	u_quad_t sticks;

	uvmexp.syscalls++;	/* XXXSMP */
	l = curlwp;
	p = l->l_proc;

	sticks = p->p_sticks;
	l->l_md.md_tf = tf;

#ifdef FPU_DEBUG
	save_fpu(tf);
#endif
	new = handle_new(tf, &code);

	if ((error = getargs(p, tf, &code, &callp, &args)) != 0)
		goto bad;

	KERNEL_PROC_LOCK(l);
	if ((error = trace_enter(l, code, code, NULL, args.i)) != 0) {
		KERNEL_PROC_UNLOCK(l);
		goto out;
	}

	rval[0] = 0;
	rval[1] = tf->tf_out[1];

        /* Lock the kernel if the syscall isn't MP-safe. */
	if (callp->sy_flags & SYCALL_MPSAFE) {
		KERNEL_PROC_UNLOCK(l);
		error = (*callp->sy_call)(l, &args, rval);
	} else {
		error = (*callp->sy_call)(l, &args, rval);
		KERNEL_PROC_UNLOCK(l);
	}

out:
	switch (error) {
	case 0:
		/* Note: fork() does not return here in the child */
		tf->tf_out[0] = rval[0];
		tf->tf_out[1] = rval[1];
		if (new) {
			/* jmp %g2 (or %g7, deprecated) on success */
			i = tf->tf_global[new & SYSCALL_G2RFLAG ? 2 : 7];
			if (i & 3) {
				error = EINVAL;
				goto bad;
			}
		} else {
			/* old system call convention: clear C on success */
			tf->tf_psr &= ~PSR_C;	/* success */
			i = tf->tf_npc;
		}
		tf->tf_pc = i;
		tf->tf_npc = i + 4;
		break;

	case ERESTART:
	case EJUSTRETURN:
		/* nothing to do */
		break;

	default:
	bad:
		if (p->p_emul->e_errno)
			error = p->p_emul->e_errno[error];
		tf->tf_out[0] = error;
		tf->tf_psr |= PSR_C;	/* fail */
		i = tf->tf_npc;
		tf->tf_pc = i;
		tf->tf_npc = i + 4;
		break;
	}

	trace_exit(l, code, args.i, rval, error);

	userret(l, pc, sticks);
	share_fpu(l, tf);
}

/*
 * Process the tail end of a fork() for the child.
 */
void
child_return(void *arg)
{
	struct lwp *l = arg;
#ifdef KTRACE
	struct proc *p;
#endif

	/*
	 * Return values in the frame set by cpu_fork().
	 */
	KERNEL_PROC_UNLOCK(l);
	userret(l, l->l_md.md_tf->tf_pc, 0);
#ifdef KTRACE
	p = l->l_proc;
	if (KTRPOINT(p, KTR_SYSRET)) {
		KERNEL_PROC_LOCK(l);
		ktrsysret(l,
			  (p->p_flag & P_PPWAIT) ? SYS_vfork : SYS_fork, 0, 0);
		KERNEL_PROC_UNLOCK(l);
	}
#endif
}

/*
 * XXX This is a terrible name.
 */
void
upcallret(struct lwp *l)
{

	KERNEL_PROC_UNLOCK(l);
	userret(l, l->l_md.md_tf->tf_pc, 0);
}

/*
 * Start a new LWP
 */
void
startlwp(void *arg)
{
	int err;
	ucontext_t *uc = arg;
	struct lwp *l = curlwp;

	err = cpu_setmcontext(l, &uc->uc_mcontext, uc->uc_flags);
#if DIAGNOSTIC
	if (err) {
		printf("Error %d from cpu_setmcontext.", err);
	}
#endif
	pool_put(&lwp_uc_pool, uc);

	KERNEL_PROC_UNLOCK(l);
	userret(l, l->l_md.md_tf->tf_pc, 0);
}
