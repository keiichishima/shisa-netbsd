/*	$NetBSD: ex_source.c,v 1.10 2005/02/12 12:53:23 aymeric Exp $	*/

/*-
 * Copyright (c) 1992, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 * Copyright (c) 1992, 1993, 1994, 1995, 1996
 *	Keith Bostic.  All rights reserved.
 *
 * See the LICENSE file for redistribution information.
 */

#include "config.h"

#include <sys/cdefs.h>
#ifndef lint
#if 0
static const char sccsid[] = "@(#)ex_source.c	10.12 (Berkeley) 8/10/96";
#else
__RCSID("$NetBSD: ex_source.c,v 1.10 2005/02/12 12:53:23 aymeric Exp $");
#endif
#endif /* not lint */

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/stat.h>

#include <bitstring.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../common/common.h"

/*
 * ex_source -- :source file
 *	Execute ex commands from a file.
 *
 * PUBLIC: int ex_source __P((SCR *, EXCMD *));
 */
int
ex_source(sp, cmdp)
	SCR *sp;
	EXCMD *cmdp;
{
	struct stat sb;
	int fd, len;
	char *bp, *name;

	name = cmdp->argv[0]->bp;
	if ((fd = open(name, O_RDONLY, 0)) < 0 || fstat(fd, &sb))
		goto err;

	/*
	 * XXX
	 * I'd like to test to see if the file is too large to malloc.  Since
	 * we don't know what size or type off_t's or size_t's are, what the
	 * largest unsigned integral type is, or what random insanity the local
	 * C compiler will perpetrate, doing the comparison in a portable way
	 * is flatly impossible.  So, put an fairly unreasonable limit on it,
	 * I don't want to be dropping core here.
	 */
#define	MEGABYTE	1048576
	if (sb.st_size > MEGABYTE) {
		errno = ENOMEM;
		goto err;
	}

	MALLOC(sp, bp, char *, (size_t)sb.st_size + 1);
	if (bp == NULL) {
		(void)close(fd);
		return (1);
	}
	bp[sb.st_size] = '\0';

	/* Read the file into memory. */
	len = read(fd, bp, (int)sb.st_size);
	(void)close(fd);
	if (len == -1 || len != sb.st_size) {
		if (len != sb.st_size)
			errno = EIO;
		free(bp);
err:		msgq_str(sp, M_SYSERR, name, "%s");
		return (1);
	}

	/* Put it on the ex queue. */
	return (ex_run_str(sp, name, bp, (size_t)sb.st_size, 1, 1));
}
