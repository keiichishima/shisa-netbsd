#	$NetBSD: Makefile,v 1.5 2003/11/28 05:14:18 simonb Exp $

.include <bsd.own.mk>

TOP=		${NETBSDSRCDIR}/gnu

.include "${.CURDIR}/../common/Makefile.inc"
.include "${.CURDIR}/arch/${MACHINE_ARCH}/defs.mk"

PROG=		gprof
SRCS=		${G_gprof_OBJECTS:.o=.c}
CPPFLAGS+=	-I${.CURDIR}/arch/${MACHINE_ARCH}

LDADD=		-lintl
DPADD=		${LIBINTL}

BFDOBJ!=	cd ${TOP}/${BFDSUBDIR}/libbfd && ${PRINTOBJDIR}
LDADD+=		-L${BFDOBJ} -lbfd
.if ${MKPICLIB} != "no"
DPADD+=		${BFDOBJ}/libbfd_pic.a
.else
DPADD+=		${BFDOBJ}/libbfd.a
.endif

IBERTYOBJ!=	cd ${TOP}/${BFDSUBDIR}/libiberty && ${PRINTOBJDIR}
LDADD+=		-L${IBERTYOBJ} -liberty
DPADD+=		${IBERTYOBJ}/libiberty.a

TEXINFO=	${G_TEXINFOS}

.PATH: ${DIST}/gprof

.include <bsd.prog.mk>
.include <bsd.info.mk>
