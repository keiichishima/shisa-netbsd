#	$NetBSD: Makefile,v 1.8 2004/09/16 09:35:38 he Exp $

.include "../Makefile.inc"

.PATH:	$S/compat/aoutm68k

CPPFLAGS+=	-DSYSVSEM -DSYSVMSG -DSYSVSHM -DCOMPAT_AOUT_M68K -DCOMPAT_43
CPPFLAGS+=	-nostdinc
AFLAGS=		-x assembler-with-cpp -traditional-cpp -D_LOCORE -D_KERNEL \
		-I. -I${S}

KMOD=	compat_aoutm68k

SRCS=	lkminit_emul.c
SRCS+=	aoutm68k_exec.c aoutm68k_stat.c aoutm68k_sysent.c

.include <bsd.kmod.mk>
