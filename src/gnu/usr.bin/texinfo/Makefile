#	$NetBSD: Makefile,v 1.20 2004/07/18 16:34:45 tron Exp $

.include <bsd.own.mk>

SUBDIR=		common .WAIT info infokey install-info makeinfo texi2dvi texindex

.if ${MKSHARE} != "no"
FILESDIR=	/usr/share/misc
FILES=		epsf.tex texinfo.cat texinfo.dtd texinfo.tex texinfo.xsl
VERSION!=	sed -n "s/^\#define VERSION \"\(.*\)\"/\1/p" ${NETBSDSRCDIR}/gnu/usr.bin/texinfo/common/config.h

texinfo.cat: texinfo-cat.in
	sed 's/__VERSION__/${VERSION}/g' ${.ALLSRC} >${.TARGET}

realall:	texinfo.cat

CLEANFILES+=	texinfo.cat
.endif

.if ${MKNLS} != "no"
.for F in cs.gmo da.gmo de.gmo de_AT.gmo eo.gmo fr.gmo he.gmo hr.gmo ja.gmo \
	nb.gmo nl.gmo no.gmo pl.gmo ro.gmo ru.gmo sv.gmo tr.gmo zh_CN.gmo zh_TW.gmo
FILES+=		${F}
FILESDIR_${F}=  /usr/share/locale/${F:.gmo=}/LC_MESSAGES
FILESNAME_${F}= texinfo.mo
.endfor
# XXX: Do we want to install the following files too?
#FILES+= 	txi-cs.tex txi-de.tex txi-en.tex txi-es.tex txi-fr.tex txi-it.tex \
#		txi-nl.tex txi-no.tex txi-pt.tex txi-tr.tex
.endif

MAN=		info.5 texinfo.5

TEXINFO=	info-stnd.texi info.texi texinfo.txi

DIST=		${NETBSDSRCDIR}/gnu/dist
IDIST=		${DIST}/texinfo
INFOFLAGS=	-I${IDIST}/doc

.PATH: ${IDIST} ${IDIST}/doc ${IDIST}/po ${IDIST}/util

.include <bsd.info.mk>
.include <bsd.prog.mk>
.include <bsd.subdir.mk>
