#	$NetBSD: Makefile.inc,v 1.4 2006/03/15 17:35:18 kleink Exp $

# gdtoa sources
.PATH: ${.CURDIR}/gdtoa
CPPFLAGS+=-I${.CURDIR}/gdtoa

CPPFLAGS+=-DNO_FENV_H

# machine-dependent directory must provide the following:
# 	arith.h gd_qnan.h
CPPFLAGS+=-I${ARCHDIR}/gdtoa

# machine-dependent Makefile must define names of modules to use for
# float conversions:
# 	strtof(): strtof{,_vaxf}.c
# and for long double conversions on platforms with extended-precision
# format, if supported (strtod.c will define an alias otherwise):
#	strtold(): strtold_p{x,xL,Q}.c
#	helpers:   strtop{x,xL,Q}.c
.include "${ARCHDIR}/gdtoa/Makefile.inc"

# public interfaces
SRCS+=	strtod.c

# private interfaces interfacing to libc
SRCS+=	dtoa.c

# local locking implementation
SRCS+=	gdtoa_locks.c

# private interfaces
SRCS+=	dmisc.c \
	gethex.c gmisc.c \
	hd_init.c hexnan.c \
	misc.c \
	smisc.c strtodg.c sum.c \
	ulp.c

# wrappers
SRCS+=	_strtof.c _strtold.c
#SRCS+=	_hdtoa.c _ldtoa.c

# XXX no hexadecimal floating-point string conversion for VAX FP yet
.if ${MACHINE_ARCH} != "vax"
SRCS+=	strtord.c
.endif

# XXX Workaround for gcc-3.3.3 optimsation bug on sh5
.if ${MACHINE_CPU} == "sh5"
COPTS.strtod.c=	-O1
.endif
