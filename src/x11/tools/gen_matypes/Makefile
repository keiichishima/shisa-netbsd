#	$NetBSD: Makefile,v 1.1.8.1 2005/04/07 11:18:20 tron Exp $

NOMAN=	1

.include <bsd.own.mk>

.PATH:	${X11SRCDIR.xc}/extras/Mesa/src/mesa/x86

HOSTPROG=	gen_matypes
HOST_CPPFLAGS+=	-I${DESTDIR}${X11INCDIR} \
		-I${X11SRCDIR.xc}/lib/GL/include \
		-I${X11SRCDIR.xc}/extras/Mesa/include \
		-I${X11SRCDIR.xc}/extras/Mesa/src/mesa \
		-I${X11SRCDIR.xc}/extras/Mesa/src/mesa/main \
		-I${X11SRCDIR.xc}/extras/Mesa/src/mesa/glapi

.include <bsd.hostprog.mk>
