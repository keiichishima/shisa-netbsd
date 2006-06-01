/*	$NetBSD: debuglog.c,v 1.5 2003/08/20 09:50:16 martin Exp $	*/

/*-
 * Copyright (c) 2001 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Nathan J. Williams.
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

#include <err.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>

#include "pthread.h"
#include "pthread_int.h"


void pthread__debuglog_read(int, int);
void pthread__debuglog_rawdump(void);

void usage(void);


static struct pthread_msgbuf* debugbuf;

int
main(int argc, char *argv[])
{
	int ch;
	extern int optind;
	extern char *optarg;

	int follow, initialize, raw, keep;

	follow = 0;
	initialize = 0;
	raw = 0;
	keep = 0;

	while ((ch = getopt(argc, argv, "rfik")) != -1)
		switch (ch) {           
		case 'k':
			keep = 1;
			break;
		case 'r':
			raw = 1;
			break;
		case 'f': 
			follow = 1;
			break;
		case 'i':
			initialize = 1;
			break;

		default:
			usage();
			/* NOTREACHED */
		}
	
	argc -= optind;
	argv += optind;

	if (argc != 0)
		usage();

	debugbuf = pthread__debuglog_init(initialize);

	if (raw)
		pthread__debuglog_rawdump();
	else
		pthread__debuglog_read(follow, !keep);
	
	return 0;
}

void
pthread__debuglog_rawdump(void)
{

	printf("Debug log raw dump\n");
	printf("Magic: %06x\n", debugbuf->msg_magic);
	printf("Size: %06llx\n", (long long)debugbuf->msg_bufs);
	printf("Read start: %ld   Write start: %ld\n\n",
	    (long)debugbuf->msg_bufr, (long)debugbuf->msg_bufw);

	fwrite(&debugbuf->msg_bufc[0], debugbuf->msg_bufs, 1, stdout);
}

void
pthread__debuglog_read(int follow, int trunc)
{

	int readp, writep;

	do {

		readp = debugbuf->msg_bufr;
		writep = debugbuf->msg_bufw;

		if (readp < writep) {
			fwrite(&debugbuf->msg_bufc[readp], writep - readp,
			    1, stdout);
		} else if (readp > writep) {
			fwrite(&debugbuf->msg_bufc[readp], 
			    debugbuf->msg_bufs - readp, 1, stdout);
			fwrite(&debugbuf->msg_bufc[0], writep, 1, stdout); 
		}
		
		if (trunc)
			debugbuf->msg_bufr = writep;

		if (follow)
			sleep(1);
	} while (follow);
}




void usage()
{

  fprintf(stderr,"usage: debuglog [-fi]\n");
  exit(1);
}
