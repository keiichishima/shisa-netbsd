#	$NetBSD: Makefile,v 1.2 2007/11/21 15:39:34 jmmv Exp $

NOMAN=  	# defined

.include <bsd.own.mk>

TESTSDIR=	${TESTSBASE}/util/df

TESTS_SH=	t_df

BINDIR=		${TESTSDIR}
PROG=		h_df
.PATH:  	${NETBSDSRCDIR}/bin/df
.PATH:  	${NETBSDSRCDIR}/bin/csh
SRCS=		df.c strpct.c getmntinfo.c

# Pass -DINTREE to make to test using humanize_number.c in source tree
# directly instead of the one in libc.
.if defined(INTREE)
.PATH:		${NETBSDSRCDIR}/lib/libc/gen
CPPFLAGS+=	-I${NETBSDSRCDIR}/lib/libc/include
SRCS+=		humanize_number.c
.endif

.include <bsd.test.mk>
