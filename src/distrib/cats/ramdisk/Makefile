#	$NetBSD: Makefile,v 1.7 2005/01/18 12:38:45 lukem Exp $

.include <bsd.own.mk>
.include "${NETBSDSRCDIR}/distrib/common/Makefile.distrib"

IMAGE=		ramdisk.fs
IMAGESIZE=	2100k
MAKEFS_FLAGS=	-f 14

WARNS=		1

CRUNCHBIN=	ramdiskbin
LISTS=		${.CURDIR}/list ${DISTRIBDIR}/common/list.sysinst
MTREECONF=	${DISTRIBDIR}/common/mtree.common
IMAGEENDIAN=	le
MAKEDEVTARGETS=	ramdisk
IMAGEDEPENDS=	${CRUNCHBIN} \
		disktab.preinstall dot.hdprofile dot.profile termcap.mini \
		${NETBSDSRCDIR}/etc/group ${NETBSDSRCDIR}/etc/master.passwd \
		${NETBSDSRCDIR}/etc/netconfig ${NETBSDSRCDIR}/etc/protocols \
		${NETBSDSRCDIR}/etc/services

.if ${USE_INET6} != "no"
LISTS+=		${.CURDIR}/list.inet6
.endif

# Use stubs to eliminate some large stuff from libc
HACKSRC=	${DISTRIBDIR}/utils/libhack
.include	"${HACKSRC}/Makefile.inc"
${CRUNCHBIN}:	libhack.o

.include "${DISTRIBDIR}/common/Makefile.crunch"
.include "${DISTRIBDIR}/common/Makefile.makedev"
.include "${DISTRIBDIR}/common/Makefile.image"

release:

.include <bsd.prog.mk>
