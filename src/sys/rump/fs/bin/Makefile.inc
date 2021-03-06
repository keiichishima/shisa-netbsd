#	$NetBSD: Makefile.inc,v 1.2 2007/08/14 15:56:15 pooka Exp $
#

NOMAN=		1
WARNS?=		4

CPPFLAGS+=	-I${NETBSDSRCDIR}/sys/rump/fs/lib/libp2k -I${NETBSDSRCDIR}/sys
.ifndef RUMPFS_NOFSLIB
LDADD+=		${RUMPFSLD_P2K} ${RUMPFSLD_UKFS} ${LIBUTIL} ${LIBPUFFS}
.endif
LDADD+=		-L${LIBRUMPDIR} -lrump

RUMPKERNEL=		no
INSTALL_FILE=		: not today

.include "${.CURDIR}/../../Makefile.rumpfs"
.include "${NETBSDSRCDIR}/sys/rump/Makefile.rump"
