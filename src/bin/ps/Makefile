#	$NetBSD: Makefile,v 1.28 2002/06/19 08:11:55 jdolecek Exp $
#	@(#)Makefile	8.1 (Berkeley) 6/2/93

PROG=		ps
SRCS=		fmt.c keyword.c nlist.c print.c ps.c
DPADD=		${LIBM} ${LIBKVM}
LDADD=		-lm -lkvm

CWARNFLAGS+=	-Wno-format-y2k

.include <bsd.prog.mk>
