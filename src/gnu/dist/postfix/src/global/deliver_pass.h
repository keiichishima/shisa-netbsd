/*	$NetBSD: deliver_pass.h,v 1.1.1.3 2004/05/31 00:24:29 heas Exp $	*/

#ifndef _DELIVER_PASS_H_INCLUDED_
#define _DELIVER_PASS_H_INCLUDED_

/*++
/* NAME
/*	deliver_pass 3h
/* SUMMARY
/*	deliver request pass_through
/* SYNOPSIS
/*	#include <deliver_pass.h>
/* DESCRIPTION
/* .nf

 /*
  * Global library.
  */
#include <deliver_request.h>
#include <mail_proto.h>

 /*
  * External interface.
  */
extern int deliver_pass(const char *, const char *, DELIVER_REQUEST *, const char *, const char *, long);
extern int deliver_pass_all(const char *, const char *, DELIVER_REQUEST *);

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
