/*	$NetBSD: valid_hostname.h,v 1.1.1.3 2004/05/31 00:25:01 heas Exp $	*/

#ifndef _VALID_HOSTNAME_H_INCLUDED_
#define _VALID_HOSTNAME_H_INCLUDED_

/*++
/* NAME
/*	valid_hostname 3h
/* SUMMARY
/*	validate hostname
/* SYNOPSIS
/*	#include <valid_hostname.h>
/* DESCRIPTION
/* .nf

 /* External interface */

#define VALID_HOSTNAME_LEN	255	/* RFC 1035 */
#define VALID_LABEL_LEN		63	/* RFC 1035 */

#define DONT_GRIPE		0
#define DO_GRIPE		1

extern int valid_hostname(const char *, int);
extern int valid_hostaddr(const char *, int);
extern int valid_hostliteral(const char *, int);

/* LICENSE
/* .ad
/* .fi
/*	The Secure Mailer license must be distributed with this software.
/* AUTHOR(S)
/*	Wietse Venema
/*	IBM T.J. Watson Research
/*	P.O. Box 704
/*	Yorktown Heights, NY 10598, USA
/*--*/

#endif
