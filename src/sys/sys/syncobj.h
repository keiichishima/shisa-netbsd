/*	$NetBSD: syncobj.h,v 1.3 2007/02/27 15:07:28 yamt Exp $	*/

/*-
 * Copyright (c) 2007 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Andrew Doran.
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

#if !defined(_SYS_SYNCOBJ_H_)
#define	_SYS_SYNCOBJ_H_

struct lwp;

typedef volatile const void *wchan_t;

#if defined(_KERNEL)

/*
 * Synchronisation object operations set.
 */
typedef struct syncobj {
	u_int	sobj_flag;
	void	(*sobj_unsleep)(struct lwp *);
	void	(*sobj_changepri)(struct lwp *, pri_t);
	void	(*sobj_lendpri)(struct lwp *, pri_t);
	struct lwp *(*sobj_owner)(wchan_t);
} syncobj_t;

struct lwp *syncobj_noowner(wchan_t);

#define	SOBJ_SLEEPQ_SORTED	0x01
#define	SOBJ_SLEEPQ_FIFO	0x02

extern syncobj_t	sched_syncobj;
extern syncobj_t	mutex_syncobj;
extern syncobj_t	rw_syncobj;

#endif /* defined(_KERNEL) */

#endif /* !_SYS_SYNCOBJ_H_ */
