/*	$NetBSD: buffer.c,v 1.2 2004/12/06 13:16:36 wiz Exp $	*/
/*
 * Copyright (c) 2002, 2003 Niels Provos <provos@citi.umich.edu>
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
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
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

#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include "event.h"

struct evbuffer *
evbuffer_new(void)
{
	struct evbuffer *buffer;
	
	buffer = calloc(1, sizeof(struct evbuffer));

	return (buffer);
}

void
evbuffer_free(struct evbuffer *buffer)
{
	if (buffer->orig_buffer != NULL)
		free(buffer->orig_buffer);
	free(buffer);
}

/* 
 * This is a destructive add.  The data from one buffer moves into
 * the other buffer.
 */

#define SWAP(x,y) do { \
	(x)->buffer = (y)->buffer; \
	(x)->orig_buffer = (y)->orig_buffer; \
	(x)->misalign = (y)->misalign; \
	(x)->totallen = (y)->totallen; \
	(x)->off = (y)->off; \
} while (0)

int
evbuffer_add_buffer(struct evbuffer *outbuf, struct evbuffer *inbuf)
{
	int res;

	/* Short cut for better performance */
	if (outbuf->off == 0) {
		struct evbuffer tmp;
		size_t oldoff = inbuf->off;

		/* Swap them directly */
		SWAP(&tmp, outbuf);
		SWAP(outbuf, inbuf);
		SWAP(inbuf, &tmp);

		/* 
		 * Optimization comes with a price; we need to notify the
		 * buffer if necessary of the changes. oldoff is the amount
		 * of data that we tranfered from inbuf to outbuf
		 */
		if (inbuf->off != oldoff && inbuf->cb != NULL)
			(*inbuf->cb)(inbuf, oldoff, inbuf->off, inbuf->cbarg);
		if (oldoff && outbuf->cb != NULL)
			(*outbuf->cb)(outbuf, 0, oldoff, outbuf->cbarg);
		
		return (0);
	}

	res = evbuffer_add(outbuf, inbuf->buffer, inbuf->off);
	if (res == 0)
		evbuffer_drain(inbuf, inbuf->off);

	return (res);
}

int
evbuffer_add_printf(struct evbuffer *buf, char *fmt, ...)
{
	int res = -1;
	char *msg;
	va_list ap;

	va_start(ap, fmt);

	if (vasprintf(&msg, fmt, ap) == -1)
		goto end;
	
	res = strlen(msg);
	if (evbuffer_add(buf, msg, res) == -1)
		res = -1;
	free(msg);

end:
	va_end(ap);

	return (res);
}

/* Reads data from an event buffer and drains the bytes read */

int
evbuffer_remove(struct evbuffer *buf, void *data, size_t datlen)
{
	int nread = datlen;
	if (nread >= buf->off)
		nread = buf->off;

	memcpy(data, buf->buffer, nread);
	evbuffer_drain(buf, nread);
	
	return (nread);
}

/* Adds data to an event buffer */

static inline void
evbuffer_align(struct evbuffer *buf)
{
	memmove(buf->orig_buffer, buf->buffer, buf->off);
	buf->buffer = buf->orig_buffer;
	buf->misalign = 0;
}

/* Expands the available space in the event buffer to at least datlen */

int
evbuffer_expand(struct evbuffer *buf, size_t datlen)
{
	size_t need = buf->misalign + buf->off + datlen;

	/* If we can fit all the data, then we don't have to do anything */
	if (buf->totallen >= need)
		return (0);

	/*
	 * If the misalignment fulfills our data needs, we just force an
	 * alignment to happen.  Afterwards, we have enough space.
	 */
	if (buf->misalign >= datlen) {
		evbuffer_align(buf);
	} else {
		void *newbuf;
		size_t length = buf->totallen;

		if (length < 256)
			length = 256;
		while (length < need)
			length <<= 1;

		if (buf->orig_buffer != buf->buffer)
			evbuffer_align(buf);
		if ((newbuf = realloc(buf->buffer, length)) == NULL)
			return (-1);

		buf->orig_buffer = buf->buffer = newbuf;
		buf->totallen = length;
	}

	return (0);
}

int
evbuffer_add(struct evbuffer *buf, void *data, size_t datlen)
{
	size_t need = buf->misalign + buf->off + datlen;
	size_t oldoff = buf->off;

	if (buf->totallen < need) {
		if (evbuffer_expand(buf, datlen) == -1)
			return (-1);
	}

	memcpy(buf->buffer + buf->off, data, datlen);
	buf->off += datlen;

	if (datlen && buf->cb != NULL)
		(*buf->cb)(buf, oldoff, buf->off, buf->cbarg);

	return (0);
}

void
evbuffer_drain(struct evbuffer *buf, size_t len)
{
	size_t oldoff = buf->off;

	if (len >= buf->off) {
		buf->off = 0;
		buf->buffer = buf->orig_buffer;
		buf->misalign = 0;
		goto done;
	}

	buf->buffer += len;
	buf->misalign += len;

	buf->off -= len;

 done:
	/* Tell someone about changes in this buffer */
	if (buf->off != oldoff && buf->cb != NULL)
		(*buf->cb)(buf, oldoff, buf->off, buf->cbarg);

}

/*
 * Reads data from a file descriptor into a buffer.
 */

#define EVBUFFER_MAX_READ	4096

int
evbuffer_read(struct evbuffer *buf, int fd, int howmuch)
{
	u_char *p;
	size_t oldoff = buf->off;
	int n = EVBUFFER_MAX_READ;

	if (ioctl(fd, FIONREAD, &n) == -1)
		n = EVBUFFER_MAX_READ;

	if (howmuch < 0 || howmuch > n)
		howmuch = n;

	/* If we don't have FIONREAD, we might waste some space here */
	if (evbuffer_expand(buf, howmuch) == -1)
		return (-1);

	/* We can append new data at this point */
	p = buf->buffer + buf->off;

	n = read(fd, p, howmuch);
	if (n == -1)
		return (-1);
	if (n == 0)
		return (0);

	buf->off += n;

	/* Tell someone about changes in this buffer */
	if (buf->off != oldoff && buf->cb != NULL)
		(*buf->cb)(buf, oldoff, buf->off, buf->cbarg);

	return (n);
}

int
evbuffer_write(struct evbuffer *buffer, int fd)
{
	int n;

	n = write(fd, buffer->buffer, buffer->off);
	if (n == -1)
		return (-1);
	if (n == 0)
		return (0);
	evbuffer_drain(buffer, n);

	return (n);
}

u_char *
evbuffer_find(struct evbuffer *buffer, u_char *what, size_t len)
{
	size_t remain = buffer->off;
	u_char *search = buffer->buffer;
	u_char *p;

	while ((p = memchr(search, *what, remain)) != NULL && remain >= len) {
		if (memcmp(p, what, len) == 0)
			return (p);

		search = p + 1;
		remain = buffer->off - (size_t)(search - buffer->buffer);
	}

	return (NULL);
}

void evbuffer_setcb(struct evbuffer *buffer,
    void (*cb)(struct evbuffer *, size_t, size_t, void *),
    void *cbarg)
{
	buffer->cb = cb;
	buffer->cbarg = cbarg;
}
