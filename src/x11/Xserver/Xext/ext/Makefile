#	$NetBSD: Makefile,v 1.4.4.1 2005/04/06 21:01:43 tron Exp $

LIB=		ext
SRCS=		EVI.c appgroup.c bigreq.c cup.c dpms.c fontcache.c mitmisc.c \
		panoramiX.c panoramiXSwap.c panoramiXprocs.c sampleEVI.c \
		saver.c security.c shape.c shm.c sleepuntil.c sync.c \
		xcmisc.c xf86bigfont.c xprint.c xres.c xtest.c xtest1dd.c \
		xtest1di.c xvdisp.c xvmain.c xvmc.c dmx.c

.if ${MACHINE} != "sparc"
SRCS+=	xf86vmode.c xf86misc.c xf86dga.c xf86dga2.c
.endif

CONFIGFILES=	SecurityPolicy
FILESDIR=	${X11ETCDIR}/xserver

.include "../../Makefile.serverlib"
.include "../Makefile.ext"
