#	$NetBSD: Makefile,v 1.8 2005/01/10 02:58:59 lukem Exp $
#	from: @(#)Makefile	5.2 (Berkeley) 5/11/90

.include <bsd.own.mk>

PROG=	rpcinfo
MAN=	rpcinfo.8
SRCS=	rpcinfo.c rpc_generic.c

LIBCDIR=	${NETBSDSRCDIR}/lib/libc
LIBCRPCDIR=	${LIBCDIR}/rpc
LIBCINCLUDE=	${LIBCDIR}/include

CPPFLAGS+=	-I${LIBCRPCDIR} -I${LIBCINCLUDE} -DPORTMAP

.if (${USE_INET6} != "no")
CPPFLAGS+=	-DINET6
.endif

.PATH:	${LIBCRPCDIR}

.include <bsd.prog.mk>
