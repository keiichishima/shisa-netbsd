# $NetBSD: Makefile,v 1.4 2004/07/30 15:04:19 wiz Exp $

# $FreeBSD: src/gnu/usr.bin/groff/src/utils/hpftodit/Makefile,v 1.4 2003/05/01 13:22:20 ru Exp $

DIST_SUBDIR=	src/utils/hpftodit

PROG_CXX=	hpftodit
SRCS=		hpftodit.cpp hpuni.cpp
DPADD+=		${LIBGROFF} ${LIBM}
LDADD+=		${LIBGROFF} -lm
CLEANFILES=	${MAN}

.include <bsd.prog.mk>
