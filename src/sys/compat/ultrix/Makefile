#	$NetBSD: Makefile,v 1.4 2005/02/26 23:51:29 perry Exp $

.include <bsd.sys.mk>		# for HOST_SH

DEP=	syscalls.conf syscalls.master ../../kern/makesyscalls.sh
OBJS=	ultrix_sysent.c ultrix_syscalls.c ultrix_syscall.h ultrix_syscallargs.h

${OBJS}: ${DEP}
	${HOST_SH} ../../kern/makesyscalls.sh syscalls.conf syscalls.master

all: ${OBJS}

.include <bsd.kinc.mk>
