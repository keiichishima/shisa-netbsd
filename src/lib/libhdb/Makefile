# $NetBSD: Makefile,v 1.18 2005/01/10 03:11:17 lukem Exp $

NOLINT=		# defined

.include <bsd.own.mk>

DIST=		${NETBSDSRCDIR}/crypto/dist
.PATH:		${DIST}/heimdal/lib/hdb

WARNS?=		1

LIB=		hdb

.if ${USETOOLS} != "yes"
COMPILEETOBJ!=		cd ${NETBSDSRCDIR}/lib/libcom_err/compile_et && ${PRINTOBJDIR}
TOOL_COMPILE_ET=	${COMPILEETOBJ}/compile_et

ASN1COMPILEOBJ!=	cd ${NETBSDSRCDIR}/lib/libasn1/asn1_compile && ${PRINTOBJDIR}
TOOL_ASN1_COMPILE=	${ASN1COMPILEOBJ}/asn1_compile
.endif

hdb_err.c hdb_err.h: hdb_err.et ${TOOL_COMPILE_ET}
	${TOOL_COMPILE_ET} ${DIST}/heimdal/lib/hdb/hdb_err.et

ERR_FILES=	hdb_err.c

gen_files=	asn1_Key.x asn1_Event.x asn1_HDBFlags.x asn1_hdb_entry.x \
		asn1_Salt.x asn1_GENERATION.x

DPSRCS=		hdb_asn1.h hdb_err.h

BUILT_SOURCES=	${gen_files:.x=.c} hdb_err.c

SRCS=					\
	    common.c			\
	    db.c			\
	    db3.c			\
	    hdb-ldap.c			\
	    hdb.c			\
	    keytab.c			\
	    mkey.c			\
	    ndbm.c			\
	    print.c			\
	    $(BUILT_SOURCES)

INCS=		hdb.h hdb_err.h hdb_asn1.h hdb-protos.h hdb-private.h
INCSDIR=	/usr/include/krb5

asn1_files hdb_asn1.hx: hdb.asn1 ${TOOL_ASN1_COMPILE}
	${TOOL_ASN1_COMPILE} ${DIST}/heimdal/lib/hdb/hdb.asn1 hdb_asn1

ASN1!=		cd ${NETBSDSRCDIR}/lib/libasn1 && ${PRINTOBJDIR}
KRB5!=		cd ${NETBSDSRCDIR}/lib/libkrb5 && ${PRINTOBJDIR}

CPPFLAGS+= -I.					\
	 -I${DIST}/heimdal/lib/hdb		\
	 -I${ASN1}				\
	 -I${KRB5}				\
	 -I${DIST}/heimdal/lib/krb5		\
	 -I${DIST}/heimdal/lib/asn1		\
	 -I${DIST}/heimdal/lib/com_err		\
	 -I${NETBSDSRCDIR}/include/heimdal	\
	 -I${NETBSDSRCDIR}/lib/libroken		\
	 -I${DIST}/heimdal/lib/roken		\
	 -DHAVE_CONFIG_H

.if (${USE_INET6} != "no")
CPPFLAGS+=-DHAVE_IPV6
.endif

${gen_files} hdb_asn1.h: asn1_files

hdb_asn1.h: hdb_asn1.hx
${SRCS}: hdb_err.h hdb_asn1.h

CLEANFILES=	${gen_files} ${BUILT_SOURCES} \
		hdb_asn1.h hdb_asn1.hx asn1_files hdb_err.h

.include <bsd.lib.mk>

.SUFFIXES: .hx .x

.x.c:
	@cmp -s $< $@ 2> /dev/null || cp $< $@

.hx.h:
	@cmp -s $< $@ 2> /dev/null || cp $< $@

