# $NetBSD: Makefile,v 1.1.1.2 2004/01/02 15:00:25 cjep Exp $

PROG=	grep
SRCS=	binary.c file.c grep.c mmfile.c queue.c util.c

LINKS=  ${BINDIR}/grep ${BINDIR}/egrep	\
	${BINDIR}/grep ${BINDIR}/fgrep	\
	${BINDIR}/grep ${BINDIR}/zgrep	\
	${BINDIR}/grep ${BINDIR}/zegrep	\
	${BINDIR}/grep ${BINDIR}/zfgrep

MLINKS= grep.1 egrep.1	\
	grep.1 fgrep.1	\
	grep.1 zgrep.1	\
	grep.1 zegrep.1	\
	grep.1 zfgrep.1

LDADD=  -lz

WARNS=2

.include <bsd.prog.mk>
