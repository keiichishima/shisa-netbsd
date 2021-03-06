/*	$NetBSD: compat___fts13.c,v 1.3 2006/03/30 18:24:49 christos Exp $	*/

#include "namespace.h"
#include <sys/cdefs.h>
#include <dirent.h>

#define __LIBC12_SOURCE__

__warn_references(__fts_children13,
    "warning: reference to compatibility __fts_children13();"
    " include <fts.h> for correct reference")
__warn_references(__fts_close13,
    "warning: reference to compatibility __fts_close13();"
    " include <fts.h> for correct reference")
__warn_references(__fts_open13,
    "warning: reference to compatibility __fts_open13();"
    " include <fts.h> for correct reference")
__warn_references(__fts_read13,
    "warning: reference to compatibility __fts_read13();"
    " include <fts.h> for correct reference")
__warn_references(__fts_set13,
    "warning: reference to compatibility __fts_set13();"
    " include <fts.h> for correct reference")

#include <sys/stat.h>
#include <compat/sys/stat.h>

#define __fts_stat_t	struct stat13
#define	__fts_nlink_t	nlink_t
#define	__fts_ino_t	u_int32_t
#define	__fts_length_t	u_short

#define stat		__stat13
#define lstat		__lstat13
#define fstat		__fstat13

#undef fts_children
#define fts_children __fts_children13
#undef fts_close
#define fts_close __fts_close13
#undef fts_open
#define fts_open  __fts_open13
#undef fts_read
#define fts_read __fts_read13
#undef fts_set
#define fts_set __fts_set13

#include <fts.h>
#include <compat/include/fts.h>

#define	__FTS_COMPAT_TAILINGSLASH
#define	__FTS_COMPAT_LENGTH

#include "gen/fts.c"
