/* $NetBSD: hp300.c,v 1.4 2003/11/13 08:19:43 dsl Exp $ */

/*-
 * Copyright (c) 2003 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by David Laight.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#if HAVE_NBTOOL_CONFIG_H
#include "nbtool_config.h"
#endif

#include <sys/cdefs.h>
#if !defined(__lint)
__RCSID("$NetBSD: hp300.c,v 1.4 2003/11/13 08:19:43 dsl Exp $");
#endif /* !__lint */

/* We need the target disklabel.h, not the hosts one..... */
#ifdef HAVE_NBTOOL_CONFIG_H
#include "nbtool_config.h"
#undef __HAVE_OLD_DISKLABEL	/* host's <machine/types.h> may define this */  
#include "../../sys/arch/hp300/include/disklabel.h"                            
#include "../../sys/sys/disklabel.h"                                           
#else                                                                          
#include <sys/disklabel.h>
#endif
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/param.h>

#include <assert.h>
#include <err.h>
#include <md5.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "installboot.h"

int
hp300_setboot(ib_params *params)
{
	int		retval;
	uint8_t		*bootstrap;
	ssize_t		rv;
	struct partition *boot;
	struct hp300_lifdir *lifdir;
	int		offset;
	int		i;
	uint		secsize;
	uint64_t	boot_size, boot_offset;
	char		label_buf[DEV_BSIZE];
	struct disklabel *label = (void *)label_buf;

	assert(params != NULL);
	assert(params->fsfd != -1);
	assert(params->filesystem != NULL);
	assert(params->s1fd != -1);
	assert(params->stage1 != NULL);

	retval = 0;
	bootstrap = MAP_FAILED;

	if (params->flags & IB_APPEND) {
		if (!S_ISREG(params->fsstat.st_mode)) {
			warnx(
		    "`%s' must be a regular file to append a bootstrap",
			    params->filesystem);
			goto done;
		}
		boot_offset = roundup(params->fsstat.st_size, HP300_SECTSIZE);
	} else {
		/*
		 * The bootstrap can be well over 8k, and must go into a BOOT
		 * partition. Read NetBSD label to locate BOOT partition.
		 */
		if (pread(params->fsfd, label, DEV_BSIZE, 2 * DEV_BSIZE)
								!= DEV_BSIZE) {
			warn("reading disklabel");
			goto done;
		}
		/* And a quick validation - must be a big-endian label */
		secsize = be32toh(label->d_secsize);
		if (label->d_magic != be32toh(DISKMAGIC) ||
		    label->d_magic2 != be32toh(DISKMAGIC) ||
		    secsize == 0 || secsize & (secsize - 1) ||
		    be32toh(label->d_npartitions) > MAXMAXPARTITIONS) {
			warnx("Invalid disklabel in %s", params->filesystem);
			goto done;
		}

		i = be32toh(label->d_npartitions);
		for (boot = label->d_partitions; ; boot++) {
			if (--i < 0) {
				warnx("No BOOT partition");
				goto done;
			}
			if (boot->p_fstype == FS_BOOT)
				break;
		}
		boot_size = be32toh(boot->p_size) * (uint64_t)secsize;
		boot_offset = be32toh(boot->p_offset) * (uint64_t)secsize;

		/*
		 * We put the entire LIF file into the BOOT partition even when
		 * it doesn't start at the beginning of the disk.
		 *
		 * Maybe we ought to be able to take a binary file and add
		 * it to the LIF filesystem.
		 */
		if (boot_size < params->s1stat.st_size) {
			warn("BOOT partition too small (%llu < %llu)",
				(unsigned long long)boot_size,
				(unsigned long long)params->s1stat.st_size);
			goto done;
		}
	}

	bootstrap = mmap(NULL, params->s1stat.st_size, PROT_READ | PROT_WRITE,
			    MAP_PRIVATE, params->s1fd, 0);
	if (bootstrap == MAP_FAILED) {
		warn("mmaping `%s'", params->stage1);
		goto done;
	}

	/* Relocate files, sanity check LIF directory on the way */
	lifdir = (void *)(bootstrap + HP300_SECTSIZE * 2);
	for (i = 0; i < 8; lifdir++, i++) {
		int addr = be32toh(lifdir->dir_addr);
		int limit = (params->s1stat.st_size - 1) / HP300_SECTSIZE + 1;
		if (addr + be32toh(lifdir->dir_length) > limit) {
			warnx("LIF entry %d larger (%d %d) than LIF file",
				i,  addr + be32toh(lifdir->dir_length), limit);
			goto done;
		}
		if (addr != 0 && boot_offset != 0)
			lifdir->dir_addr = htobe32(addr + boot_offset
							    / HP300_SECTSIZE);
	}

	if (params->flags & IB_NOWRITE) {
		retval = 1;
		goto done;
	}

	/* Write LIF volume header and directory to sectors 0 and 1 */
	rv = pwrite(params->fsfd, bootstrap, 1024, 0);
	if (rv != 1024) {
		if (rv == -1)
			warn("Writing `%s'", params->filesystem);
		else
			warnx("Writing `%s': short write", params->filesystem);
		goto done;
	}

	/* Write files to BOOT partition */
	offset = boot_offset <= HP300_SECTSIZE * 16 ? HP300_SECTSIZE * 16 : 0;
	i = roundup(params->s1stat.st_size, secsize) - offset;
	rv = pwrite(params->fsfd, bootstrap + offset, i, boot_offset + offset);
	if (rv != i) {
		if (rv == -1)
			warn("Writing boot filesystem of `%s'",
				params->filesystem);
		else
			warnx("Writing boot filesystem of `%s': short write",
				params->filesystem);
		goto done;
	}

	retval = 1;

 done:
	if (bootstrap != MAP_FAILED)
		munmap(bootstrap, params->s1stat.st_size);
	return retval;
}
