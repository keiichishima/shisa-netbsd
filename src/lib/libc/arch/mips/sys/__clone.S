/*	$NetBSD: __clone.S,v 1.6 2003/02/08 04:01:53 simonb Exp $	*/

/*-
 * Copyright (c) 2001 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jason R. Thorpe.
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

#include <sys/errno.h>

#include "SYS.h"

#ifdef WEAK_ALIAS
WEAK_ALIAS(clone, __clone)
#endif

/*
 * int __clone(int (*fn)(void *), void *stack, int flags, void *arg);
 */
LEAF(__clone)
	PIC_PROLOGUE(__clone, t9)

	/*
	 * Sanity checks: func and stack may not be NULL.
	 */
	li	v0, EINVAL
	beq	a0, zero, 8f
	beq	a1, zero, 8f

	/*
	 * We need to be able to get at the func and arg arguments
	 * in the child.  Luckily, we have a convenient place to
	 * do this: the child's stack.
	 */
	subu	a1, a1, (2 * SZREG)
	REG_S	a0, 0(a1)
	REG_S	a3, SZREG(a1)

	/*
	 * The system call expects (flags, stack).
	 */
	move	a0, a2
	SYSTRAP(__clone)
	bne	a3, zero, 8f

	bne	v1, zero, 9f	# v1 (rv[1]) == 0, parent, child pid in v0

	j	ra		# parent return

8:	PIC_CALL(__cerror, t9)
	/* NOTREACHED */

9:	/*
	 * Child: Fetch the function and argument from the new stack,
	 * and create a frame so that the child can be safely called.
	 *
	 * There are already register slots on the stack from above.
	 * We take 4 register slots more.  The lower 4 accommodate o32
	 * argument save area.  The next higher is used to save GP (if
	 * ABICALLS).  The highest is unused.
	 */
	REG_L	t9, 0(sp)
	REG_L	a0, SZREG(sp)
	subu	sp, sp, (4 * SZREG)

#ifdef __ABICALLS__
	.cprestore (4 * SZREG)
#endif

	/* Call the clone's entry point. */
	jal	t9

	/* Pass the return value to _exit(). */
	move	a0, v0
	PIC_CALL(_exit, t9)

	/* NOTREACHED */
END(__clone)
