#	$NetBSD: Makefile,v 1.3 2004/11/14 11:26:48 yamt Exp $
#	$OpenBSD: Makefile,v 1.17 2004/07/16 23:44:24 frantzen Exp $

.include <bsd.own.mk>		# for MKDYNAMICROOT definition

PROG=	pfctl
SRCS=	pfctl.c parse.y pfctl_parser.c pf_print_state.c pfctl_altq.c
SRCS+=	pfctl_osfp.c pfctl_radix.c pfctl_table.c pfctl_qstats.c
SRCS+=	pfctl_optimize.c
CPPFLAGS+=-I${NETBSDSRCDIR}/dist/pf/sbin/pfctl
YFLAGS=
MAN=	pfctl.8

LDADD+=	-lm
DPADD+=	${LIBM}

BINDIR=		/sbin
.if (${MKDYNAMICROOT} == "no")
LDSTATIC?=	-static
.endif

.include <bsd.prog.mk>

.PATH: ${NETBSDSRCDIR}/dist/pf/sbin/pfctl
