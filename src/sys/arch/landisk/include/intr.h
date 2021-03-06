/*	$NetBSD: intr.h,v 1.1 2006/09/01 21:26:18 uwe Exp $	*/

/*-
 * Copyright (c) 2002 The NetBSD Foundation, Inc.
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

#ifndef _LANDISK_INTR_H_
#define _LANDISK_INTR_H_

#include <sh3/intr.h>

/* Number of interrupt source */
#define _INTR_N		16

/* Interrupt priority levels */
#define	IPL_BIO		10	/* block I/O */
#define	IPL_NET		11	/* network */
#define	IPL_TTY		12	/* terminal */
#define	IPL_SERIAL	13	/* serial */
#define	IPL_CLOCK	14	/* clock */
#define	IPL_HIGH	15	/* everything */

#define	splsoftclock()		_cpu_intr_raise(IPL_SOFTCLOCK << 4)
#define	splsoftnet()		_cpu_intr_raise(IPL_SOFTNET << 4)
#define	splsoftserial()		_cpu_intr_raise(IPL_SOFTSERIAL << 4)
#define	splbio()		_cpu_intr_raise(IPL_BIO << 4)
#define	splnet()		_cpu_intr_raise(IPL_NET << 4)
#define	spltty()		_cpu_intr_raise(IPL_TTY << 4)
#define	splvm()			spltty()
#define	splserial()		_cpu_intr_raise(IPL_SERIAL << 4)
#define	splclock()		_cpu_intr_raise(IPL_CLOCK << 4)
#define	splstatclock()		splclock()
#define	splsched()		splclock()
#define	splhigh()		_cpu_intr_raise(IPL_HIGH << 4)
#define	spllock()		splhigh()

#define	spl0()			_cpu_intr_resume(IPL_NONE << 4)
#define	splx(x)			_cpu_intr_resume(x)

#define	spllowersoftclock()	_cpu_intr_resume(IPL_SOFTCLOCK << 4)

void intr_init(void);
void *extintr_establish(int irq, int level, int (*func)(void *), void *arg);
void extintr_disestablish(void *ih);
void extintr_enable(void *ih);
void extintr_disable(void *ih);
void extintr_disable_by_num(int irq);

#endif /* !_LANDISK_INTR_H_ */
