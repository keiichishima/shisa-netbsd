/*	$NetBSD: extattr.h,v 1.1 2005/01/02 16:08:30 thorpej Exp $	*/

/*-
 * Copyright (c) 1999-2001 Robert N. M. Watson
 * All rights reserved.
 *
 * This software was developed by Robert Watson for the TrustedBSD Project.
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
 * FreeBSD: src/sys/sys/extattr.h,v 1.12 2003/06/04 04:04:24 rwatson Exp
 */

/*
 * Developed by the TrustedBSD Project.
 * Support for extended filesystem attributes.
 */

#ifndef _SYS_EXTATTR_H_
#define	_SYS_EXTATTR_H_

#define	EXTATTR_NAMESPACE_USER		0x00000001
#define	EXTATTR_NAMESPACE_USER_STRING	"user"
#define	EXTATTR_NAMESPACE_SYSTEM	0x00000002
#define	EXTATTR_NAMESPACE_SYSTEM_STRING	"system"

#ifdef _KERNEL

#define	EXTATTR_MAXNAMELEN	NAME_MAX
struct lwp;
struct ucred;
struct vnode;
int	extattr_check_cred(struct vnode *, int, struct ucred *,
	    struct proc *, int);

#else

#include <sys/cdefs.h>
__BEGIN_DECLS
int	extattrctl(const char *_path, int _cmd, const char *_filename,
	    int _attrnamespace, const char *_attrname);

int	extattr_delete_fd(int _fd, int _attrnamespace, const char *_attrname);
int	extattr_delete_file(const char *_path, int _attrnamespace,
	    const char *_attrname);
int	extattr_delete_link(const char *_path, int _attrnamespace,
	    const char *_attrname);
ssize_t	extattr_get_fd(int _fd, int _attrnamespace, const char *_attrname,
	    void *_data, size_t _nbytes);
ssize_t	extattr_get_file(const char *_path, int _attrnamespace,
	    const char *_attrname, void *_data, size_t _nbytes);
ssize_t	extattr_get_link(const char *_path, int _attrnamespace,
	    const char *_attrname, void *_data, size_t _nbytes);
ssize_t	extattr_list_fd(int _fd, int _attrnamespace, void *_data,
	    size_t _nbytes);
ssize_t	extattr_list_file(const char *_path, int _attrnamespace, void *_data,
	    size_t _nbytes);
ssize_t	extattr_list_link(const char *_path, int _attrnamespace, void *_data,
	    size_t _nbytes);
int	extattr_set_fd(int _fd, int _attrnamespace, const char *_attrname,
	    const void *_data, size_t _nbytes);
int	extattr_set_file(const char *_path, int _attrnamespace,
	    const char *_attrname, const void *_data, size_t _nbytes);
int	extattr_set_link(const char *_path, int _attrnamespace,
	    const char *_attrname, const void *_data, size_t _nbytes);

int	extattr_namespace_to_string(int, char **);
int	extattr_string_to_namespace(const char *, int *);
__END_DECLS

#endif /* !_KERNEL */
#endif /* !_SYS_EXTATTR_H_ */
