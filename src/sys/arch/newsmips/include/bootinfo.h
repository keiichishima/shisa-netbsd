/*	$NetBSD: bootinfo.h,v 1.3 2005/02/06 02:18:02 tsutsui Exp $	*/

/*
 * Copyright (c) 1997
 *	Matthias Drochner.  All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#define BOOTINFO_MAGIC	0xb007babe
#define BOOTINFO_SIZE	1024
#define BOOTINFO_ADDR	0x80000200

struct btinfo_common {
	int next;		/* offset of next item, or zero */
	int type;
};

#define BTINFO_MAGIC	1
#define BTINFO_SYMTAB	2
#define BTINFO_BOOTARG	3
#define BTINFO_BOOTPATH	4
#define BTINFO_SYSTYPE	5

struct btinfo_magic {
	struct btinfo_common common;
	int magic;
};

struct btinfo_symtab {
	struct btinfo_common common;
	int nsym;
	int ssym;
	int esym;
};

struct btinfo_bootarg {
	struct btinfo_common common;
	int howto;
	int bootdev;
	int maxmem;
	int sip;	/* APbus only */
};

#define BTINFO_BOOTPATH_LEN	80
struct btinfo_bootpath {
	struct btinfo_common common;
	char bootpath[BTINFO_BOOTPATH_LEN];
};

struct btinfo_systype {
	struct btinfo_common common;
	int type;
};

#ifdef _KERNEL
void *lookup_bootinfo(int);
#endif

#ifdef _STANDALONE
void bi_init(paddr_t);
void bi_add(void *, int, int);
#endif
