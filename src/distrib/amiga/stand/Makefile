#       $NetBSD: Makefile,v 1.1 2004/06/16 02:31:27 mhitch Exp $
#

MISC_FILES=	runbootblock.README
UUDECODE_FILES=	device-streams.tar.gz loadbsd loadbsd-2.14 loadbsd-2.14.gz \
		loadbsd-2.16.gz loadbsd.gz rdbinfo rdbinfo.gz runbootblock \
		xstreamtodev.gz

.include <bsd.files.mk>
.include <bsd.hostprog.mk>

release: check_RELEASEDIR .WAIT ${MISC_FILES} ${UUDECODE_FILES}
	${_MKMSG_INSTALL} ${MISC_FILES}
	cd ${.CURDIR} && \
	${HOST_INSTALL_FILE} -m ${NONBINMODE} ${MISC_FILES} \
	    ${RELEASEDIR}/${MACHINE}/installation/misc
	${_MKMSG_INSTALL} ${UUDECODE_FILES}
	cd ${.OBJDIR} && \
	${HOST_INSTALL_FILE} -m ${NONBINMODE} ${UUDECODE_FILES} \
	    ${RELEASEDIR}/${MACHINE}/installation/misc

