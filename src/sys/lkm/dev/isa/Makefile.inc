# $NetBSD: Makefile.inc,v 1.6 2007/09/11 07:41:26 xtraeme Exp $

.include "../Makefile.inc"

TEMPL_DIR?=	${.CURDIR}/../common
TEMPL_FILE?=	lkm_isa.c.tmpl

LKM_ISA_IO_PORT?=	-1
LKM_ISA_SIZE?=		0
LKM_ISA_IOMEM?=		-1
LKM_ISA_IOSIZE?=	0
LKM_ISA_IRQ?=		-1
LKM_ISA_DRQ?=		-1
LKM_ISA_DRQ2?=		-1
LKM_ISA_DRVNAME_CA?=	${LKM_ISA_DRVNAME}

CLEANFILES+=	${.CURDIR}/${TEMPL_FILE:S/.tmpl//}

DPSRCS+=	${TEMPL_FILE:S/.tmpl//}

${TEMPL_FILE:S/.tmpl//}:
	sed 	-e "s|@LKM_ISA_IO_PORT@|${LKM_ISA_IO_PORT}|g" \
		-e "s|@LKM_ISA_SIZE@|${LKM_ISA_SIZE}|g" \
		-e "s|@LKM_ISA_IOMEM@|${LKM_ISA_IOMEM}|g" \
		-e "s|@LKM_ISA_IOSIZE@|${LKM_ISA_IOSIZE}|g" \
		-e "s|@LKM_ISA_IRQ@|${LKM_ISA_IRQ}|g" \
		-e "s|@LKM_ISA_DRQ@|${LKM_ISA_DRQ}|g" \
		-e "s|@LKM_ISA_DRQ2@|${LKM_ISA_DRQ2}|g" \
		-e "s|@LKM_ISA_DRVNAME@|${LKM_ISA_DRVNAME}|g" \
		-e "s|@LKM_ISA_DRVNAME_CA@|${LKM_ISA_DRVNAME_CA}|g" \
		${TEMPL_DIR}/${TEMPL_FILE} > ${.CURDIR}/${TEMPL_FILE:S/.tmpl//}
