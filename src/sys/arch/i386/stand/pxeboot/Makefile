#	$NetBSD: Makefile,v 1.7.10.1 2005/09/15 20:43:37 tron Exp $

S=	${.CURDIR}/../../../../

BASE?= pxeboot_ia32
PROG= ${BASE}.bin
NOMAN=	# defined
NEWVERSWHAT=	"PXE Boot"
STARTFILE=	${PXESTART}
RELOC=		0x0

.if (${BASE} != "pxeboot_ia32")
.PATH.c: ${.CURDIR}/../pxeboot
.PATH.S: ${.CURDIR}/../pxeboot
.endif

SRCS= main.c dev_net.c devopen.c conf.c exec.c pxe.c pxe_call.S
# use our own nfs implementation
.PATH: ${.CURDIR}/../libsa
SRCS+= nfs.c

.if ${MACHINE} == "amd64"
LD+=  -m elf_i386
AFLAGS+=   -m32
CPUFLAGS=  -m32
LIBKERN_ARCH=i386
KERNMISCMAKEFLAGS="LIBKERN_ARCH=i386"
CPPFLAGS+= -DBOOT_ELF64
.else
CPUFLAGS=  -mcpu=i386
.endif

CPPFLAGS+= -DSLOW	# for libz

.if (${BASE} == "pxeboot_ia32")
# Various serial line configurations
CPPFLAGS+= -DSUPPORT_SERIAL=CONSDEV_PC -DDIRECT_SERIAL
#	or
#CPPFLAGS+= -DSUPPORT_SERIAL=CONSDEV_COM0KBD
#	or
#CPPFLAGS+= -DSUPPORT_SERIAL=CONSDEV_AUTO 
#	and maybe 
#CPPFLAGS+= -DDIRECT_SERIAL -DCOMCONS_KEYPRESS -DCONSPEED=57600
.endif

.if (${BASE} == "pxeboot_ia32_com0")
CPPFLAGS+= -DSUPPORT_SERIAL=CONSDEV_COM0 -DDIRECT_SERIAL
.endif

CPPFLAGS+= -DSUPPORT_BOOTP -DSUPPORT_DHCP
CPPFLAGS+= -DSUPPORT_TFTP
CPPFLAGS+= -DSUPPORT_NFS
#CPPFLAGS+= -DNFS_NOSYMLINK

CPPFLAGS+= -DPASS_MEMMAP

#CFLAGS= -O2 -fomit-frame-pointer -fno-defer-pop
CFLAGS+= -Wall -Wmissing-prototypes -Wstrict-prototypes -Wno-main

SAMISCCPPFLAGS+= -DHEAP_START=0x10000 -DHEAP_LIMIT=0x30000
SAMISCMAKEFLAGS+= SA_USE_CREAD=yes	# Read compressed kernels

CPPFLAGS+= -DPASS_BIOSGEOM
# if you don't use -DPASS_BIOSGEOM, then set I386_INCLUDE_DISK=no
#I386MISCMAKEFLAGS= I386_INCLUDE_DISK=no

.if (${BASE} == "pxeboot_ia32")
VERSIONFILE= ${.CURDIR}/version
.else
VERSIONFILE= ${.CURDIR}/../pxeboot/version
.endif

.include "../Makefile.booters"
