/*	$NetBSD: rtld_start.S,v 1.9 2002/10/05 11:59:05 mycroft Exp $	*/

/*
 * Copyright 1997 Michael L. Hitch <mhitch@montana.edu>
 * Portions copyright 2002 Charles M. Hannum <root@ihack.net>
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

#include <mips/asm.h>

.globl _C_LABEL(_rtld_relocate_nonplt_self)
.globl _C_LABEL(_rtld)

LEAF(rtld_start)
	.set	noreorder

	.cpload t9
	addu	sp, sp, -12		# adjust stack pointer
	.cprestore 0			# -> 0(sp) for gp
					# -> 4(sp) for atexit
					# -> 8(sp) for obj_main
	move	s0,a0			# save stack pointer from a0
	move	s1,a3			# save ps_strings pointer

	la	a1, 1f
	bal	1f
	 la	t9,_C_LABEL(_rtld_relocate_nonplt_self)
1:	subu	a1, ra, a1		# relocbase
	move	s2,a1
	la	a0,_DYNAMIC
	addu	t9, a1, t9
	jalr	t9
	 addu	a0, a1, a0		# &_DYNAMIC

	move	a1,s2			# relocbase
	addu	a0, sp, 4		# sp
	jal	_C_LABEL(_rtld)		# v0 = _rtld(sp, relocbase)
	 nop

	lw	a1, 4(sp)		# our atexit function
	lw	a2, 8(sp)		# obj_main entry
	addu	sp, sp,12		# readjust stack
	move	a0,s0			# stack pointer
	move	t9,v0
	jr	t9			# _start(sp, cleanup, obj);
	move	a3,s1			# restore ps_strings

END(rtld_start)

	.globl	_rtld_bind_start
	.ent	_rtld_bind_start
_rtld_bind_start:

	move	v1,gp			# save old GP
	add	t9,8			# modify T9 to point at .cpload
	.cpload	t9
	subu	sp,44			# save arguments and sp value in stack
	.cprestore 36
	sw	t7,40(sp)
	sw	a0,16(sp)
	sw	a1,20(sp)
	sw	a2,24(sp)
	sw	a3,28(sp)
	sw	s0,32(sp)
	move	s0,sp
	move	a0,t8			# symbol index
	move	a1,t7			# old RA
	move	a2,v1			# old GP
	move	a3,ra			# current RA
	jal	_C_LABEL(_rtld_bind)
	nop
	move	sp,s0
	lw	ra,40(sp)
	lw	a0,16(sp)
	lw	a1,20(sp)
	lw	a2,24(sp)
	lw	a3,28(sp)
	lw	s0,32(sp)
	addu	sp,44
	move	t9,v0
	jr	t9
	nop
	.end	_rtld_bind_start
