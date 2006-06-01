/*	$NetBSD: disklabel.h,v 1.5 2004/07/28 09:17:31 skrll Exp $	*/

/*	$OpenBSD: disklabel.h,v 1.5 2000/07/05 22:37:22 mickey Exp $	*/

/*
 * Copyright (c) 1994 Christopher G. Demetriou
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Christopher G. Demetriou.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
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
 */

#ifndef _MACHINE_DISKLABEL_H_
#define _MACHINE_DISKLABEL_H_

#define	LABELSECTOR		1		/* sector containing label */
#define	LABELOFFSET		0		/* offset of label in sector */

#define	MAXPARTITIONS		16		/* number of partitions */
#define	RAW_PART		2		/* raw partition: xx?c */

#include <sys/bootblock.h>

#include <sys/dkbad.h>
struct cpu_disklabel {
	struct hp700_lifvol lifvol;
	struct hp700_lifdir lifdir[HP700_LIF_NUMDIR];
	struct dkbad bad;			/* To make wd(4) happy */
};

#endif /* _MACHINE_DISKLABEL_H_ */
