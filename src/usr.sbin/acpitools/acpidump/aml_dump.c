/*	$NetBSD: aml_dump.c,v 1.1 2007/01/14 04:36:13 christos Exp $	*/

/*-
 * Copyright (c) 2000 Mitsuru IWASAKI <iwasaki@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	Id: aml_dump.c,v 1.3 2000/08/08 14:12:21 iwasaki Exp 
 *	$FreeBSD: src/usr.sbin/acpi/acpidump/aml_dump.c,v 1.4 2001/10/22 17:25:25 iwasaki Exp $
 */
#include <sys/cdefs.h>
__RCSID("$NetBSD: aml_dump.c,v 1.1 2007/01/14 04:36:13 christos Exp $");

#include <sys/param.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <acpi_common.h>
#include "acpidump.h"

char	*aml_dumpfile = NULL;

void
aml_dump(struct ACPIsdt *dsdp)
{
	int	fd;
	mode_t	mode;

	if (aml_dumpfile == NULL) {
		return;
	}

	mode = (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	fd = open(aml_dumpfile, O_WRONLY | O_CREAT | O_TRUNC, mode);
	if (fd == -1) {
		return;
	}
	write(fd, dsdp, SIZEOF_SDT_HDR);
	write(fd, dsdp->body, dsdp->len - SIZEOF_SDT_HDR);
	close(fd);
}

