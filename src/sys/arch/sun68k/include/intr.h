/*	$NetBSD: intr.h,v 1.3 2005/01/23 17:27:04 chs Exp $	*/

/*
 * Copyright (c) 2001 Matt Fredette.
 * Copyright (c) 1998 Matt Thomas.
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
 * 3. The name of the company nor the names of the authors may be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _SUN68K_INTR_H_
#define _SUN68K_INTR_H_

#include <sys/queue.h>
#include <m68k/psl.h>

/*
 * Interrupt levels.  Right now these correspond to real
 * hardware levels, but I don't think anything counts on
 * that (yet?).
 */
#define _IPL_SOFT_LEVEL1	1
#define _IPL_SOFT_LEVEL2	2
#define _IPL_SOFT_LEVEL3	3
#define _IPL_SOFT_LEVEL_MIN	1
#define _IPL_SOFT_LEVEL_MAX	3
#define IPL_SOFTNET  		_IPL_SOFT_LEVEL1
#define IPL_SOFTCLOCK		_IPL_SOFT_LEVEL1
#define IPL_SOFTSERIAL		_IPL_SOFT_LEVEL3
#define	IPL_BIO			2
#define	IPL_NET			3
#define	IPL_CLOCK		5
#define	IPL_SERIAL		6

#ifdef _KERNEL
LIST_HEAD(sh_head, softintr_handler);

struct softintr_head {
	int shd_ipl;
	struct sh_head shd_intrs;
};

struct softintr_handler {
	struct softintr_head *sh_head;
	LIST_ENTRY(softintr_handler) sh_link;
	void (*sh_func)(void *);
	void *sh_arg;
	int sh_pending;
};

extern void softintr_init(void);
extern void *softintr_establish(int, void (*)(void *), void *);
extern void softintr_disestablish(void *);

static __inline void
softintr_schedule(void *arg)
{
	struct softintr_handler * const sh = arg;
	if (sh->sh_pending == 0) {
		sh->sh_pending = 1;
		isr_soft_request(sh->sh_head->shd_ipl);
	}
}

/* These connect interrupt handlers. */
typedef int (*isr_func_t)(void *);
extern void isr_add_autovect(isr_func_t, void *, int);
extern void isr_add_vectored(isr_func_t, void *, int, int);
extern void isr_add_custom(int, void *);

/*
 * Define inline functions for PSL manipulation.
 * These are as close to macros as one can get.
 * When not optimizing gcc will call the locore.s
 * functions by the same names, so breakpoints on
 * these functions will work normally, etc.
 * (See the GCC extensions info document.)
 */

static __inline int _getsr(void);

/* Get current sr value. */
static __inline int
_getsr(void)
{
	int rv;

	__asm __volatile ("clrl %0; movew %%sr,%0" : "=&d" (rv));
	return (rv);
}

/*
 * The rest of this is sun68k specific, because other ports may
 * need to do special things in spl0() (i.e. simulate SIR).
 * Suns have a REAL interrupt register, so spl0() and splx(s)
 * have no need to check for any simulated interrupts, etc.
 */

#define spl0()  _spl0()		/* we have real software interrupts */
#define splx(x)	_spl(x)

/* IPL used by soft interrupts: netintr(), softclock() */
#define	spllowersoftclock() spl1()
#define splsoftclock()  splraise1()
#define splsoftnet()    splraise1()

/* Highest block device (strategy) IPL. */
#define splbio()        splraise2()

/* Highest network interface IPL. */
#define splnet()        splraise3()

/* Highest tty device IPL. */
#define spltty()        splraise4()

/*
 * Requirement: imp >= (highest network, tty, or disk IPL)
 * This is used mostly in the VM code.
 * Note that the VM code runs at spl7 during kernel
 * initialization, and later at spl0, so we have to 
 * use splraise to avoid enabling interrupts early.
 */
#define splvm()         _splraise(PSL_S|PSL_IPL4)

/* Intersil or Am9513 clock hardware interrupts (hard-wired at 5) */
#define splclock()      splraise5()
#define splstatclock()  splclock()

/* Zilog Serial hardware interrupts (hard-wired at 6) */
#define splzs()		spl6()

/* Block out all interrupts (except NMI of course). */
#define splhigh()       spl7()
#define splsched()      spl7()
#define spllock()	spl7()

/* This returns true iff the spl given is spl0. */
#define	is_spl0(s)	(((s) & PSL_IPL7) == 0)

#endif	/* _KERNEL */

#endif	/* _SUN68K_INTR_H */
