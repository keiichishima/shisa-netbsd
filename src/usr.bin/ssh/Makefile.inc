#	$NetBSD: Makefile.inc,v 1.11.2.1 2005/09/02 12:42:10 tron Exp $

.include <bsd.own.mk>

SSHDIST?= ${NETBSDSRCDIR}/crypto/dist/ssh

CPPFLAGS+=-I${SSHDIST} -DHAVE_LOGIN_CAP -DHAVE_MMAP -DHAVE_OPENPTY
.PATH: ${SSHDIST}

LDADD+=	-lssh -lcrypto -lcrypt -lz
DPADD+=	${LIBSSH} ${LIBCRYPTO} ${LIBCRYPT} ${LIBZ}

.if (${USE_PAM} != "no")
CPPFLAGS+=-DUSE_PAM
.endif

.if (${USE_KERBEROS} != "no")

CPPFLAGS+=-DGSSAPI -I${DESTDIR}/usr/include/gssapi
CPPFLAGS+=-DKRB5 -I${DESTDIR}/usr/include/krb5

.if ${MKKERBEROS4} != "no"
CPPFLAGS+=-DKRB4 -DAFS -I${DESTDIR}/usr/include/kerberosIV
.endif

.endif

CPPFLAGS+=-DSUPPORT_UTMP -DSUPPORT_UTMPX
CPPFLAGS+=-DLIBWRAP

.if (${USE_SKEY} != "no")
CPPFLAGS+=-DSKEY
.endif

.if exists(${.CURDIR}/../../Makefile.inc)
.include "${.CURDIR}/../../Makefile.inc"
.endif
