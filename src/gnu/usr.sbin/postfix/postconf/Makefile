#	$NetBSD: Makefile,v 1.11 2004/02/08 22:28:27 lukem Exp $

# XXX Note we aren't building ../conf/main.cf.default
# The shipped makefiles construct it using postconf -d after building
# postconf. It isn't entirely clear how to deal with that in a cross
# build environment, and the .default file isn't that useful to
# the user anyway.

NOMAN=	# defined

.include <bsd.own.mk>

PROG=	postconf

DIST=	${NETBSDSRCDIR}/gnu/dist/postfix/src/${PROG}
.PATH:	${DIST}

BINDIR= ${PFIX_SBINDIR}           

PSRCS=	postconf.c
GENSRCS=bool_table.h bool_vars.h int_table.h int_vars.h str_table.h \
	str_vars.h time_table.h time_vars.h raw_table.h raw_vars.h

SRCS=	${PSRCS} ${GENSRCS}
DPSRCS=	${GENSRCS}

CLEANFILES+= ${GENSRCS}

CPPFLAGS+= -I.

DPADD+= ${LIBPGLOBAL} ${LIBPUTIL}
LDADD+= ${LIBPGLOBAL} ${LIBPUTIL}

# XXX Note that there is no ${AWK} variable in our build system. This
# is probably a deficiency.
${GENSRCS}: postconf-hdrs.stamp
CLEANFILES+= postconf-hdrs.stamp
postconf-hdrs.stamp: ${DIST}/../global/mail_params.h ${DIST}/../global/mail_params.c
	${_MKMSG_CREATE} ${GENSRCS}
	rm -f ${.TARGET}
	awk -f ${DIST}/extract.awk ${DIST}/../*/*.c
	touch ${.TARGET}

.include <bsd.prog.mk>
