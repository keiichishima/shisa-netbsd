#	$NetBSD: Makefile,v 1.2 2004/03/06 11:44:41 rtr Exp $

.include <bsd.own.mk>

KMOD=	mga
MKMAN=	no

.PATH:	${X11SRCDIR.xc}/programs/Xserver/hw/xfree86/os-support/shared/drm/kernel
SRCS=	mga_dma.c mga_irq.c mga_state.c mga_warp.c

.PATH:	${X11SRCDIR.xc}/programs/Xserver/hw/xfree86/os-support/bsd/drm/kernel
SRCS+=	mga_drv.c

DPSRCS=	opt_drm.h

opt_drm.h:
	echo > opt_drm.h
	echo "#define DRM_DEBUG 1" >> opt_drm.h
	echo "#define DRM_LINUX 1" >> opt_drm.h

CPPFLAGS+=	-I${X11SRCDIR.xc}/programs/Xserver/hw/xfree86/os-support/bsd/drm/kernel \
		-I${X11SRCDIR.xc}/programs/Xserver/hw/xfree86/os-support/shared/drm/kernel

.include <bsd.kmod.mk>
