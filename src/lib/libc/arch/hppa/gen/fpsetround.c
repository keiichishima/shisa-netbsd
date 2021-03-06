/*	$NetBSD: fpsetround.c,v 1.1 2004/07/24 19:09:29 chs Exp $	*/

/*	$OpenBSD: fpsetround.c,v 1.3 2002/10/21 18:41:05 mickey Exp $	*/

/*
 * Written by Miodrag Vallat.  Public domain
 */

#include <sys/types.h>
#include <ieeefp.h>

fp_rnd
fpsetround(fp_rnd rnd_dir)
{
	uint64_t fpsr;
	fp_rnd old;

	__asm__ __volatile__("fstd %%fr0,0(%1)" : "=m" (fpsr) : "r" (&fpsr));
	old = (fpsr >> 41) & 0x03;
	fpsr = (fpsr & 0xfffff9ff00000000LL) |
	    ((uint64_t)(rnd_dir & 0x03) << 41);
	__asm__ __volatile__("fldd 0(%0),%%fr0" : : "r" (&fpsr));
	return (old);
}
