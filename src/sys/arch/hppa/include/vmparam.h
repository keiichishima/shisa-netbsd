/*	$NetBSD: vmparam.h,v 1.6 2004/07/18 23:21:35 chs Exp $	*/

/*	$OpenBSD: vmparam.h,v 1.17 2001/09/22 18:00:09 miod Exp $	*/

/* 
 * Copyright (c) 1988-1994, The University of Utah and
 * the Computer Systems Laboratory at the University of Utah (CSL).
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software is hereby
 * granted provided that (1) source code retains these copyright, permission,
 * and disclaimer notices, and (2) redistributions including binaries
 * reproduce the notices in supporting documentation, and (3) all advertising
 * materials mentioning features or use of this software display the following
 * acknowledgement: ``This product includes software developed by the
 * Computer Systems Laboratory at the University of Utah.''
 *
 * THE UNIVERSITY OF UTAH AND CSL ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS
 * IS" CONDITION.  THE UNIVERSITY OF UTAH AND CSL DISCLAIM ANY LIABILITY OF
 * ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * CSL requests users of this software to return to csl-dist@cs.utah.edu any
 * improvements that they make and grant CSL redistribution rights.
 *
 * 	Utah $Hdr: vmparam.h 1.16 94/12/16$
 */

#ifndef _HPPA_VMPARAM_H_
#define _HPPA_VMPARAM_H_

/*
 * Machine dependent constants for HP PA
 */

/*
 * We use 4K pages on the HP PHA.  Override the PAGE_* definitions
 * to be compile-time constants.
 */
#define	PAGE_SHIFT	12
#define	PAGE_SIZE	(1 << PAGE_SHIFT)
#define	PAGE_MASK	(PAGE_SIZE - 1)

/*
 * USRSTACK is the top (end) of the user stack.
 */
#define	USRSTACK	0x70000000		/* Start of user stack */
#define	SYSCALLGATE	0xC0000000		/* syscall gateway page */

/* Alignment requirement for a uspace. */
#define	USPACE_ALIGN	PAGE_SIZE

/*
 * Virtual memory related constants, all in bytes
 */
#ifndef MAXTSIZ
#define	MAXTSIZ		(0x40000000)		/* max text size */
#endif
#ifndef DFLDSIZ
#define	DFLDSIZ		(16*1024*1024)		/* initial data size limit */
#endif
#ifndef MAXDSIZ
#define	MAXDSIZ		(USRSTACK-MAXTSIZ)	/* max data size */
#endif
#ifndef	DFLSSIZ
#define	DFLSSIZ		(512*1024)		/* initial stack size limit */
#endif
#ifndef	MAXSSIZ
#define	MAXSSIZ		(256*1024*1024)	/* max stack size */
#endif

#ifndef USRIOSIZE
#define	USRIOSIZE	((2*HPPA_PGALIAS)/PAGE_SIZE)	/* 2mb */
#endif

/*
 * PTEs for system V style shared memory.
 * This is basically slop for kmempt which we actually allocate (malloc) from.
 */
#ifndef SHMMAXPGS
#define SHMMAXPGS	((1024*1024*10)/PAGE_SIZE)	/* 10mb */
#endif

/*
 * The time for a process to be blocked before being very swappable.
 * This is a number of seconds which the system takes as being a non-trivial
 * amount of real time.  You probably shouldn't change this;
 * it is used in subtle ways (fractions and multiples of it are, that is, like
 * half of a ``long time'', almost a long time, etc.)
 * It is related to human patience and other factors which don't really
 * change over time.
 */
#define	MAXSLP 		20

/* user/kernel map constants */
#define	VM_MIN_ADDRESS		((vaddr_t)0)
#define	VM_MAXUSER_ADDRESS	((vaddr_t)0xc0000000)
#define	VM_MAX_ADDRESS		VM_MAXUSER_ADDRESS
#define	VM_MIN_KERNEL_ADDRESS	((vaddr_t)0)
#define	VM_MAX_KERNEL_ADDRESS	((vaddr_t)0xf0000000)

/* virtual sizes (bytes) for various kernel submaps */
#define VM_PHYS_SIZE		(USRIOSIZE*PAGE_SIZE)

#define	VM_PHYSSEG_MAX	8	/* this many physmem segments */
#define	VM_PHYSSEG_STRAT	VM_PSTRAT_BIGFIRST

#define	VM_PHYSSEG_NOADD	/* XXX until uvm code is fixed */

#define	VM_NFREELIST		1
#define	VM_FREELIST_DEFAULT	0

#endif	/* _HPPA_VMPARAM_H_ */
