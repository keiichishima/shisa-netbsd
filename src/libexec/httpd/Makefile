#	$NetBSD: Makefile,v 1.4 2007/10/18 17:43:02 he Exp $
#
#	$eterna: Makefile,v 1.26 2005/09/27 20:09:20 mrg Exp $

# compile-time options are:
#	DEBUG			/* include debugging support */
#	NO_USER_SUPPORT		/* don't support /~user requests */
#	NO_CGIBIN_SUPPORT	/* don't support cgi-bin requests */
#	NO_DIRINDEX_SUPPORT	/* don't support directory indexing */
#	NO_DAEMON_MODE		/* don't support daemon mode */
#	NO_DYNAMIC_CONTENT	/* don't support dynamic content updates */
#	NO_SSL_SUPPORT		/* don't support ssl (https) */
#	DO_HTPASSWD		/* support .htpasswd files */
COPTS+=	-DDO_HTPASSWD
PROG=	httpd
MAN=	httpd.8
SRCS=	bozohttpd.c ssl-bozo.c auth-bozo.c cgi-bozo.c daemon-bozo.c \
	tilde-luzah-bozo.c dir-index-bozo.c content-bozo.c

WARNS=	3

CRYPTOLIBS=	-lcrypto -lssl -lcrypt
CRYPTODEPS=	${LIBCRYPTO} ${LIBSSL} ${LIBCRYPT}

# if not defining -DNO_SSL_SUPPORT
LDADD=	-lssl -lcrypto -lcrypt
DPADD=	${LIBSSL} ${LIBCRYPTO} ${LIBCRYPT}

.include <bsd.prog.mk>
