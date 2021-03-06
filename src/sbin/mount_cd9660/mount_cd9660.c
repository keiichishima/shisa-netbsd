/*	$NetBSD: mount_cd9660.c,v 1.20 2005/02/05 14:49:36 xtraeme Exp $	*/

/*
 * Copyright (c) 1992, 1993, 1994
 *      The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley
 * by Pace Willisson (pace@blitz.com).  The Rock Ridge Extension
 * Support code is derived from software contributed to Berkeley
 * by Atsushi Murai (amurai@spec.co.jp).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      @(#)mount_cd9660.c	8.7 (Berkeley) 5/1/95
 */

#include <sys/cdefs.h>
#ifndef lint
__COPYRIGHT("@(#) Copyright (c) 1992, 1993, 1994\n\
        The Regents of the University of California.  All rights reserved.\n");
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)mount_cd9660.c	8.7 (Berkeley) 5/1/95";
#else
__RCSID("$NetBSD: mount_cd9660.c,v 1.20 2005/02/05 14:49:36 xtraeme Exp $");
#endif
#endif /* not lint */

#include <sys/param.h>
#include <sys/mount.h>

#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <util.h>

#include <isofs/cd9660/cd9660_mount.h>

#include <mntopts.h>

static const struct mntopt mopts[] = {
	MOPT_STDOPTS,
	MOPT_UPDATE,
	MOPT_GETARGS,
	{ "extatt", 0, ISOFSMNT_EXTATT, 1 },
	{ "gens", 0, ISOFSMNT_GENS, 1 },
	{ "maplcase", 1, ISOFSMNT_NOCASETRANS, 1 },
	{ "nrr", 0, ISOFSMNT_NORRIP, 1 },
	{ "rrip", 1, ISOFSMNT_NORRIP, 1 },
	{ "joliet", 1, ISOFSMNT_NOJOLIET, 1 },
	{ "rrcaseins", 0, ISOFSMNT_RRCASEINS, 1 },
	{ NULL }
};

int	mount_cd9660(int argc, char **argv);
static void	usage(void);

#ifndef MOUNT_NOMAIN
int
main(int argc, char **argv)
{
	return mount_cd9660(argc, argv);
}
#endif

int
mount_cd9660(int argc, char **argv)
{
	struct iso_args args;
	int ch, mntflags, opts;
	char *dev, *dir, canon_dev[MAXPATHLEN], canon_dir[MAXPATHLEN];

	mntflags = opts = 0;
	while ((ch = getopt(argc, argv, "egijo:r")) != -1)
		switch (ch) {
		case 'e':
			/* obsolete, retained for compatibility only, use
			 * -o extatt */
			opts |= ISOFSMNT_EXTATT;
			break;
		case 'g':
			/* obsolete, retained for compatibility only, use
			 * -o gens */
			opts |= ISOFSMNT_GENS;
			break;
		case 'j':
			/* obsolete, retained fo compatibility only, use
			 * -o nojoliet */
			opts |= ISOFSMNT_NOJOLIET;
			break;
		case 'o':
			getmntopts(optarg, mopts, &mntflags, &opts);
			break;
		case 'r':
			/* obsolete, retained for compatibility only, use
			 * -o norrip */
			opts |= ISOFSMNT_NORRIP;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc != 2)
		usage();

	dev = argv[0];
	dir = argv[1];

	if (realpath(dev, canon_dev) == NULL)        /* Check device path */
		err(1, "realpath %s", dev);
	if (strncmp(dev, canon_dev, MAXPATHLEN)) {
		warnx("\"%s\" is a relative path.", dev);
		dev = canon_dev;
		warnx("using \"%s\" instead.", dev);
	}

	if (realpath(dir, canon_dir) == NULL)        /* Check mounton path */
		err(1, "realpath %s", dir);
	if (strncmp(dir, canon_dir, MAXPATHLEN)) {
		warnx("\"%s\" is a relative path.", dir);
		dir = canon_dir;
		warnx("using \"%s\" instead.", dir);
	}

#define DEFAULT_ROOTUID	-2
	/*
	 * ISO 9660 filesystems are not writable.
	 */
	mntflags |= MNT_RDONLY;
	args.export.ex_flags = MNT_EXRDONLY;
	args.fspec = dev;
	args.export.ex_root = DEFAULT_ROOTUID;
	args.flags = opts;

	if (mount(MOUNT_CD9660, dir, mntflags, &args) < 0)
		err(1, "%s on %s", dev, dir);
	if (mntflags & MNT_GETARGS) {
		char buf[2048];
		(void)snprintb(buf, sizeof(buf), ISOFSMNT_BITS, args.flags);
		printf("%s\n", buf);
	}
	exit(0);
}

static void
usage(void)
{
	(void)fprintf(stderr,
		"usage: mount_cd9660 [-o options] special node\n");
	exit(1);
}
