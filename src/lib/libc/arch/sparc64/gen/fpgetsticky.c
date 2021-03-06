/*	$NetBSD: fpgetsticky.c,v 1.2 2002/01/13 21:45:50 thorpej Exp $	*/

/*
 * Written by J.T. Conklin, Apr 10, 1995
 * Public domain.
 */

#include <sys/cdefs.h>

#include "namespace.h"

#include <ieeefp.h>

#ifdef __weak_alias
__weak_alias(fpgetsticky,_fpgetsticky)
#endif

fp_except
fpgetsticky()
{
	int x;

	__asm__("st %%fsr,%0" : "=m" (*&x));
	return (x >> 5) & 0x1f;
}
