# $NetBSD: Makefile,v 1.3 2003/06/30 18:47:49 wiz Exp $

# $FreeBSD: src/gnu/usr.bin/groff/src/devices/grotty/Makefile,v 1.4 2003/05/01 13:22:19 ru Exp $

DIST_SUBDIR=	src/devices/grotty

PROG_CXX=	grotty
SRCS=		tty.cpp
DPADD+=		${LIBDRIVER} ${LIBGROFF} ${LIBM}
LDADD+=		${LIBDRIVER} ${LIBGROFF} -lm
CLEANFILES=	${MAN}

.include <bsd.prog.mk>
