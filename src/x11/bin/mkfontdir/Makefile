#	$NetBSD: Makefile,v 1.3.8.1 2005/04/06 20:47:27 tron Exp $

.include <bsd.own.mk>

.PATH:	${X11SRCDIR.xc}/programs/mkfontdir

MAN=	mkfontdir.1

PROG=	mkfontdir
CPPFLAGS+=-I${X11SRCDIR.xc}/lib/font/include
CPPFLAGS+=-I${X11SRCDIR.xc}/include/fonts
LDADD+= -lXfont -lfreetype -lfntstubs 
DPADD+= ${LIBXFONT} ${LIBFREETYPE} ${LIBFNTSTUBS}
LDADD+=	-lXt -lXext -lX11 -lz -lm
DPADD+=	${LIBXT} ${LIBXEXT} ${LIBX11} ${LIBZ} ${LIBM}
#CPPSCRIPTS=	mkfontdir
#CPPSCRIPTFLAGS=	-DBINDIR=${X11BINDIR}
#SCRIPTS=	mkfontdir

.include <bsd.x11.mk>
.include <bsd.prog.mk>
