#	$NetBSD: Makefile,v 1.13 2006/11/09 00:34:49 scw Exp $

.include <bsd.own.mk>
.include "${NETBSDSRCDIR}/distrib/common/Makefile.distrib"

RAMDISKDIR!=	cd ${.CURDIR}/../ramdisk && ${PRINTOBJDIR}
RAMDISK=	${RAMDISKDIR}/ramdisk.fs

.if ${MACHINE_ARCH} == "arm"
# Little endian platforms.
MDSETTARGETS=		ADI_BRH_INSTALL		${RAMDISK}	-	\
			CP3100_INSTALL		${RAMDISK}	-	\
			GUMSTIX_INSTALL		${RAMDISK}	-	\
			INTEGRATOR_INSTALL	${RAMDISK}	-	\
			IQ80310_INSTALL		${RAMDISK}	-	\
			IQ80321_INSTALL		${RAMDISK}	-	\
			TS7200_INSTALL		${RAMDISK}	- 	\
			TEAMASA_NPWR_INSTALL	${RAMDISK}	-	\
			GUMSTIX_INSTALL		${RAMDISK}	-
.else
# Big endian platforms.
MDSETTARGETS=		ADI_BRH_INSTALL		${RAMDISK}	-
.endif
MDSET_RELEASEDIR=	installation/instkernel

MDSET_SUFFIXES.-=	srec create-srec  bin create-bin
create-srec=		${OBJCOPY} -O srec ${.TARGET:R} ${.TARGET}
create-bin=		${OBJCOPY} -O binary ${.TARGET:R} ${.TARGET}


.include "${DISTRIBDIR}/common/Makefile.mdset"

.include <bsd.prog.mk>
