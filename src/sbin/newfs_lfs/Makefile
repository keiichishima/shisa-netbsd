#	$NetBSD: Makefile,v 1.3 2005/02/26 05:45:54 perseant Exp $
#	@(#)Makefile	8.1 (Berkeley) 6/18/93

.include <bsd.own.mk>

PROG=	newfs_lfs
SRCS=	dkcksum.c make_lfs.c lfs_cksum.c misc.c newfs.c
SRCS+=	bufcache.c vnode.c lfs.c segwrite.c
MAN=	newfs_lfs.8
.PATH:	${NETBSDSRCDIR}/sys/ufs/lfs ${NETBSDSRCDIR}/sbin/disklabel ${NETBSDSRCDIR}/sbin/fsck_lfs

FSCK_LFS= ${NETBSDSRCDIR}/sbin/fsck_lfs
DPADD=  ${LIBUTIL}
LDADD=  -lutil
CPPFLAGS+=-I${FSCK_LFS} # -DNDEBUG # -DVERBOSE_BLOCKMAP

.include <bsd.prog.mk>
