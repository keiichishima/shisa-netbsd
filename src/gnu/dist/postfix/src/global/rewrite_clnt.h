/*	$NetBSD: rewrite_clnt.h,v 1.1.1.2 2004/05/31 00:24:35 heas Exp $	*/

#ifndef _REWRITE_CLNT_H_INCLUDED_
#define _REWRITE_CLNT_H_INCLUDED_

/*++
/* NAME
/*	rewrite_clnt 3h
/* SUMMARY
/*	address rewriter client
/* SYNOPSIS
/*	#include <rewrite_clnt.h>
/* DESCRIPTION
/* .nf

 /*
  * Utility library.
  */
#include <vstring.h>

 /*
  * External interface.
  */
#define REWRITE_ADDR	"rewrite"
#define REWRITE_CANON	"canonicalize"

extern VSTRING *rewrite_clnt(const char *, const char *, VSTRING *);
extern VSTRING *rewrite_clnt_internal(const char *, const char *, VSTRING *);

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
