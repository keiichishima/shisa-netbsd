#	$NetBSD: Makefile,v 1.4 2004/09/10 07:11:26 jdolecek Exp $

.include <bsd.sys.mk>		# for HOST_SH

DEP=	syscalls.conf syscalls.master ../../kern/makesyscalls.sh
OBJS=	aoutm68k_sysent.c aoutm68k_syscalls.c \
	aoutm68k_syscall.h aoutm68k_syscallargs.h

${OBJS}: ${DEP}
	${HOST_SH} ../../kern/makesyscalls.sh syscalls.conf syscalls.master
	rm -f aoutm68k_syscalls.c	# garbage-collect, not needed

all: ${OBJS}

.include <bsd.kinc.mk>
