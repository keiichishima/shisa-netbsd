#	$NetBSD: Makefile.inc,v 1.14 2004/12/12 12:43:05 mrg Exp $

.if !defined(__MAKEFILE_INC_INCLUDED__)
__MAKEFILE_INC_INCLUDED__=1

.include <bsd.own.mk>

GNUHOSTDIST=	${DIST}
GNUCPPFLAGS=	${G_DEFS} ${G_INCLUDES}
CPPFLAGS+=	${GNUCPPFLAGS:M-D*:N-DLOCALEDIR*} ${GNUCPPFLAGS:M-I*:N-I.*} \
		-I${TOP}/${BFDSUBDIR}/libbfd/arch/${MACHINE_ARCH} \
		-I${DIST}/include -I${DIST}/bfd -I${DIST}/binutils \
		-DLOCALEDIR=\"${LOCALEDIR}\"

.if exists(${.CURDIR}/../../Makefile.inc)
.include "${.CURDIR}/../../Makefile.inc"
.endif

.endif # __MAKEFILE_INC_INCLUDED__
