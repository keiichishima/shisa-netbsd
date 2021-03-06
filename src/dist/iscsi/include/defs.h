/* $NetBSD: defs.h,v 1.4 2006/04/17 16:44:42 thorpej Exp $ */

/*
 * Copyright (c) 1999-2005 Alistair Crooks.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef DEFS_H_
#define DEFS_H_

#include "config.h"

#include <sys/types.h>
#include <sys/param.h>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NEWARRAY(type,ptr,size,where,action) do {			\
	if ((ptr = (type *) calloc(sizeof(type), (unsigned)(size))) == NULL) { \
		(void) fprintf(stderr, "%s: can't allocate %lu bytes\n", \
			where, (unsigned long)(size * sizeof(type)));	\
		action;							\
	}								\
} while( /* CONSTCOND */ 0)

#define RENEW(type,ptr,size,where,action) do {				\
	type *_newptr;							\
	if ((_newptr = (type *) realloc(ptr, sizeof(type) * (size))) == NULL) { \
		(void) fprintf(stderr, "%s: can't realloc %lu bytes\n",	\
			where, (unsigned long)(size * sizeof(type)));	\
		action;							\
	} else {							\
		ptr = _newptr;						\
	}								\
} while( /* CONSTCOND */ 0)

#define NEW(type, ptr, where, action)	NEWARRAY(type, ptr, 1, where, action)

#define FREE(ptr)	(void) free(ptr)

#define ALLOC(type, v, size, c, init, incr, where, action) do {		\
	uint32_t	_newsize = size;				\
	if (size == 0) {						\
		_newsize = init;					\
		NEWARRAY(type, v, _newsize, where ": new", action);	\
	} else if (c == size) {						\
		_newsize = size + incr;					\
		RENEW(type, v, _newsize, where ": renew", action);	\
	}								\
	size = _newsize;						\
} while( /* CONSTCOND */ 0)

/*		(void) memset(&v[size], 0x0, sizeof(type) * incr);	\*/

#define DEFINE_ARRAY(name, type)					\
typedef struct name {							\
	uint32_t	c;						\
	uint32_t	size;						\
	type	       *v;						\
} name

#ifndef ABS
#define ABS(a)		(((a) < 0) ? -(a) : (a))
#endif

#endif /* !DEFS_H_ */
