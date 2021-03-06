#	$NetBSD: Makefile.inc,v 1.1 2002/06/06 20:31:22 fredette Exp $

#	$OpenBSD: Makefile.inc,v 1.4 1999/09/14 00:46:18 mickey Exp $

SRCS+=	memchr.c memcmp.c memset.c index.c rindex.c \
	strcat.c strcmp.c strcpy.c strcspn.c strlen.c \
	strncat.c strncmp.c strncpy.c strpbrk.c strsep.c \
	strspn.c strstr.c swab.c
SRCS+=	bcmp.S bzero.S ffs.S
# NetBSD doesn't currently let archs supply a strlcpy.S.
# plus, the one that fredette made is untested.
#SRCS+=	strlcpy.S

