#	$NetBSD: Makefile.inc,v 1.8 2004/02/14 07:48:37 simonb Exp $
#	from: @(#)Makefile	5.5 (Berkeley) 5/21/91

CPPFLAGS+=	-DLIBC_SCCS -I${.CURDIR}/../common_aout
OBJS+=		crt0.o gcrt0.o scrt0.o
SRCS=		crt0.c

.include <bsd.own.mk>
.include <bsd.shlib.mk>

.if ${MKPIC} != "no" && ${SHLINKINSTALLDIR} != "/usr/libexec"
OBJS+=		lcrt0.o
.endif

realall: ${OBJS}

crt0.o: ${SRCS}
	@echo "${COMPILE.c} -DCRT0 -DDYNAMIC ${.CURDIR}/${SRCS} -o ${.TARGET}"
	@${COMPILE.c} -DCRT0 -DDYNAMIC ${.CURDIR}/${SRCS} -o ${.TARGET}.o
	@${LD} -x -r -o ${.TARGET} ${.TARGET}.o
	@rm -f ${.TARGET}.o

# dependant on crt0.o to pick up header dependencies
gcrt0.o: ${SRCS} crt0.o
	@echo "${COMPILE.c} -DMCRT0 ${.CURDIR}/${SRCS} -o ${.TARGET}"
	@${COMPILE.c} -DMCRT0 ${.CURDIR}/${SRCS} -o ${.TARGET}.o
	@${LD} -x -r -o ${.TARGET} ${.TARGET}.o
	@rm -f ${.TARGET}.o

scrt0.o: ${SRCS} crt0.o
	@echo "${COMPILE.c} -DSCRT0 ${.CURDIR}/${SRCS} -o ${.TARGET}"
	@${COMPILE.c} -DSCRT0 ${.CURDIR}/${SRCS} -o ${.TARGET}.o
	@${LD} -x -r -o ${.TARGET} ${.TARGET}.o
	@rm -f ${.TARGET}.o

# same as crt0.o modulo the dynamic linker is in /libexec
lcrt0.o: ${SRCS} crt0.o
	@echo "${COMPILE.c} -DCRT0 -DDYNAMIC -DLDSO=\"${SHLINKINSTALLDIR}/ld.so\" ${.CURDIR}/${SRCS} -o ${.TARGET}"
	@${COMPILE.c} -DCRT0 -DDYNAMIC -DLDSO=\"${SHLINKINSTALLDIR}/ld.so\" ${.CURDIR}/${SRCS} -o ${.TARGET}.o
	@${LD} -x -r -o ${.TARGET} ${.TARGET}.o
	@rm -f ${.TARGET}.o

.if make(depend)
CPPFLAGS+= -DDYNAMIC
.endif

FILES=${OBJS}
FILESDIR=${LIBDIR}

.if ${MKPIC} != "no" && ${SHLINKINSTALLDIR} != "/usr/libexec"
FILESDIR_lcrt0.o:=${SHLIBINSTALLDIR}
FILESNAME_lcrt0.o:=crt0.o
.endif

.include <bsd.prog.mk>
