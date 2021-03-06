/*	$NetBSD: select.h,v 1.21.2.1 2005/03/19 13:23:15 tron Exp $	*/

/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
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
 *	@(#)select.h	8.2 (Berkeley) 1/4/94
 */

#ifndef _SYS_SELECT_H_
#define	_SYS_SELECT_H_

#include <sys/cdefs.h>
#include <sys/featuretest.h>
#include <sys/types.h>

#ifdef _NETBSD_SOURCE
#include <sys/event.h>		/* for struct klist */

/*
 * Used to maintain information about processes that wish to be
 * notified when I/O becomes possible.
 */
struct selinfo {
	struct klist	sel_klist;	/* knotes attached to this selinfo */
	pid_t		sel_pid;	/* process to be notified */
	uint8_t		sel_collision;	/* non-zero if a collision occurred */
};
#endif /* !_NETBSD_SOURCE_ */

#ifdef _KERNEL
#include <sys/signal.h>			/* for sigset_t */

struct lwp;
struct proc;
struct timeval;

int	selcommon(struct lwp *, register_t *, int, fd_set *, fd_set *,
	    fd_set *, struct timeval *, sigset_t *);
void	selrecord(struct proc *selector, struct selinfo *);
void	selwakeup(struct selinfo *);

static __inline void
selnotify(struct selinfo *sip, long knhint)
{

	if (sip->sel_pid != 0)
		selwakeup(sip);
	KNOTE(&sip->sel_klist, knhint);
}

#else /* _KERNEL */

#include <sys/sigtypes.h>
#include <time.h>

__BEGIN_DECLS
int	pselect(int, fd_set * __restrict, fd_set * __restrict,
	    fd_set * __restrict, const struct timespec * __restrict,
	    const sigset_t * __restrict);
int	select(int, fd_set * __restrict, fd_set * __restrict,
	    fd_set * __restrict, struct timeval * __restrict);
__END_DECLS
#endif /* _KERNEL */

#endif /* !_SYS_SELECT_H_ */
