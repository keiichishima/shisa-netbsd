#	$NetBSD: Makefile,v 1.8 2003/01/03 15:34:35 lukem Exp $

SUBDIR=		ramdisk miniroot
TARGETS+=	release

.include <bsd.own.mk>
.include "${NETBSDSRCDIR}/distrib/common/Makefile.distrib"

release: check_RELEASEDIR .WAIT \
	    ${DESTDIR}/usr/mdec/netboot ${DESTDIR}/usr/mdec/tapeboot \
	    MakeBootTape MakeInstTape
	${RELEASE_INSTALL} \
	    ${DESTDIR}/usr/mdec/netboot \
	    ${RELEASEDIR}/${MACHINE}/installation/netboot
	${RELEASE_INSTALL} \
	    ${DESTDIR}/usr/mdec/tapeboot \
	    ${RELEASEDIR}/${MACHINE}/installation/tapeimage
	${RELEASE_INSTALL} \
	    ${.CURDIR}/MakeBootTape ${.CURDIR}/MakeInstTape \
	    ${RELEASEDIR}/${MACHINE}/installation/tapeimage

.include <bsd.subdir.mk>
