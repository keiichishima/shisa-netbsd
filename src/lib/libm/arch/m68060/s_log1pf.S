/* $NetBSD: s_log1pf.S,v 1.5 2000/05/16 19:07:36 is Exp $ */

/*
 * FPLSP wrapper for log1pf
 * DO NOT EDIT - this file is automatically generated.
 */

#include <machine/asm.h>

ENTRY(log1pf)
#ifdef __SVR4_ABI__
	jbra _C_LABEL(__fplsp060_0138)
#else
	movel %sp@(4),%sp@-
	jbsr _C_LABEL(__fplsp060_0138)
	fmoves %fp0,%sp@
	movel %sp@+,%d0
	rts
#endif
