#	$NetBSD: Makefile,v 1.26 2003/10/21 10:01:22 lukem Exp $

.include <bsd.own.mk>

PROG=	mset
MAN=	mset.1 map3270.5
SRCS=	astosc.c map3270.c mset.c
DPSRCS=	astosc.out

MKASTOSCDIR!=cd $(.CURDIR)/../tools/mkastosc && ${PRINTOBJDIR}
MKASTOSC=${MKASTOSCDIR}/mkastosc

${MKASTOSC}:
	@cd ${.CURDIR}/../tools/mkastosc && ${MAKE}

CLEANFILES+= astosc.out
astosc.out: ${.CURDIR}/../ctlr/hostctlr.h ${.CURDIR}/../ctlr/function.h \
	    ${.CURDIR}/../ctlr/${KBD} ${MKASTOSC}
	${_MKTARGET_CREATE}
	${MKASTOSC} \
	    ${.CURDIR}/../ctlr/hostctlr.h ${.CURDIR}/../ctlr/function.h \
	    < ${.CURDIR}/../ctlr/${KBD} > tmp
	mv -f tmp ${.TARGET}

.include <bsd.prog.mk>

.PATH: ${.CURDIR}/../api ${.CURDIR}/../ascii

astosc.o:	astosc.out
