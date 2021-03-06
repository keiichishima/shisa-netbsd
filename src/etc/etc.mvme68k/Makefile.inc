#	$NetBSD: Makefile.inc,v 1.8 2004/05/21 15:32:29 he Exp $
#
#	etc.mvme68k/Makefile.inc -- mvme68k-specific etc Makefile targets
#

KERNEL_SETS=		GENERIC VME147 VME162 VME167 VME172 VME177

BUILD_KERNELS=		RAMDISK

INSTALLATION_DIRS+=	installation/miniroot	\
			installation/netboot	\
			installation/tapeimage

snap_md_post:
.for file in netboot sboot
	${HOST_INSTALL_FILE} -m ${NONBINMODE} ${DESTDIR}/usr/mdec/${file} \
	    ${RELEASEDIR}/${MACHINE}/installation/netboot
.endfor
.for file in bootst stboot
	${HOST_INSTALL_FILE} -m ${NONBINMODE} ${DESTDIR}/usr/mdec/${file} \
	    ${RELEASEDIR}/${MACHINE}/installation/tapeimage
.endfor
	${MAKESUMS} -t ${RELEASEDIR}/${MACHINE}/installation/netboot -a
	${MAKESUMS} -t ${RELEASEDIR}/${MACHINE}/installation/tapeimage -a

