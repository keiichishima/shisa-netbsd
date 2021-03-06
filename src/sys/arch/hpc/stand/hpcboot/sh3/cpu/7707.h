/*	$NetBSD: 7707.h,v 1.3 2004/08/13 15:50:09 uch Exp $	*/

/*-
 * Copyright (c) 2001, 2002, 2004 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by UCHIYAMA Yasushi.
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

#ifndef _HPCBOOT_SH_CPU_7707_H_
#define	_HPCBOOT_SH_CPU_7707_H_

#define	SH7707_LCDAR	0xa40000c0 /* address register */
#define	SH7707_LCDDR	0xa40000c2 /* display control register */
#define	SH7707_LCDPR	0xa40000c6 /* palette register */
#define	SH7707_LCDDMR	0xa40000ce /* DMA control register */

#define	SH7707_LCDAR_LCDDMR0	0x0
#define	SH7707_LCDAR_LCDDMR1	0x1
#define	SH7707_LCDAR_LCDDMR2	0x2
#define	SH7707_LCDAR_LCDDMR3	0x3
#define	SH7707_LCDAR_LCDDMR4	0x4

#define	SH7707_CACHE_LINESZ		16
#define	SH7707_CACHE_ENTRY		128
#define	SH7707_CACHE_WAY		4	/* 2-way in RAM mode */
#define	SH7707_CACHE_SIZE						\
	(SH7707_CACHE_LINESZ * SH7707_CACHE_ENTRY * SH7707_CACHE_WAY)

#define	SH7707_CACHE_ENTRY_SHIFT	4
#define	SH7707_CACHE_ENTRY_MASK		0x000007f0
#define	SH7707_CACHE_WAY_SHIFT		11
#define	SH7707_CACHE_WAY_MASK		0x00001800

#define	SH7707_CACHE_FLUSH()						\
__BEGIN_MACRO								\
	u_int32_t __e, __w, __wa, __a;					\
									\
	for (__w = 0; __w < SH7707_CACHE_WAY; __w++) {			\
		__wa = SH3_CCA | __w << SH7707_CACHE_WAY_SHIFT;		\
		for (__e = 0; __e < SH7707_CACHE_ENTRY; __e++) {	\
			__a = __wa |(__e << SH7707_CACHE_ENTRY_SHIFT);	\
			_reg_read_4(__a) &= ~0x3; /* Clear U,V bit */	\
		}							\
	}								\
__END_MACRO

#define	SH7707_MMU_DISABLE	SH3_MMU_DISABLE

#endif // _HPCBOOT_SH_CPU_7707_H_

