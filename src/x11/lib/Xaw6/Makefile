#	$NetBSD: Makefile,v 1.5.4.1 2005/09/30 22:58:58 tron Exp $

NOCHECKVER=	yes	# we want to install an older version ...
MKLINKLIB=	no

.include <bsd.own.mk>

LIB=	Xaw
.PATH:	${X11SRCDIR.xc}/lib/${LIB}

SRCS=	Actions.c AllWidgets.c AsciiSink.c AsciiSrc.c AsciiText.c \
	Box.c Command.c Converters.c Dialog.c DisplayList.c Form.c \
	Grip.c Label.c List.c MenuButton.c MultiSink.c MultiSrc.c \
	OS.c Paned.c Panner.c Pixmap.c Porthole.c Repeater.c \
	Scrollbar.c Simple.c SimpleMenu.c Sme.c SmeBSB.c SmeLine.c \
	StripChart.c Text.c TextAction.c TextPop.c TextSink.c \
	TextSrc.c TextTr.c Toggle.c Tree.c Vendor.c Viewport.c \
	XawI18n.c XawIm.c XawInit.c

CPPFLAGS+=	-DHAS_WCHAR_H -DHAS_WCTYPE_H -DNO_WIDEC_H -DOLDXAW

LIBDPLIBS=\
	Xmu	${.CURDIR}/../Xmu \
	Xt	${.CURDIR}/../Xt \
	Xpm	${.CURDIR}/../Xpm \
	Xext	${.CURDIR}/../Xext \
	X11	${.CURDIR}/../X11/dynamic

.include <bsd.x11.mk>
.include <bsd.lib.mk>
