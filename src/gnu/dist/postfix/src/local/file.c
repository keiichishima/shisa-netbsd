/*	$NetBSD: file.c,v 1.1.1.4 2004/05/31 00:24:37 heas Exp $	*/

/*++
/* NAME
/*	file 3
/* SUMMARY
/*	mail delivery to arbitrary file
/* SYNOPSIS
/*	#include "local.h"
/*
/*	int	deliver_file(state, usr_attr, path)
/*	LOCAL_STATE state;
/*	USER_ATTR usr_attr;
/*	char	*path;
/* DESCRIPTION
/*	deliver_file() appends a message to a file, UNIX mailbox format,
/*	or qmail maildir format,
/*	with duplicate suppression. It will deliver only to non-executable
/*	regular files.
/*
/*	Arguments:
/* .IP state
/*	The attributes that specify the message, recipient and more.
/*	Attributes describing alias, include or forward expansion.
/*	A table with the results from expanding aliases or lists.
/* .IP usr_attr
/*	Attributes describing user rights and environment information.
/* .IP path
/*	The file to deliver to. If the name ends in '/', delivery is done
/*	in qmail maildir format, otherwise delivery is done in UNIX mailbox
/*	format.
/* DIAGNOSTICS
/*	deliver_file() returns non-zero when delivery should be tried again.
/* SEE ALSO
/*	defer(3)
/*	bounce(3)
/* LICENSE
/* .ad
/* .fi
/*	The Secure Mailer license must be distributed with this software.
/* AUTHOR(S)
/*	Wietse Venema
/*	IBM T.J. Watson Research
/*	P.O. Box 704
/*	Yorktown Heights, NY 10598, USA
/*--*/

/* System library. */

#include <sys_defs.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

/* Utility library. */

#include <msg.h>
#include <htable.h>
#include <vstring.h>
#include <vstream.h>
#include <deliver_flock.h>
#include <set_eugid.h>

/* Global library. */

#include <mail_copy.h>
#include <bounce.h>
#include <defer.h>
#include <sent.h>
#include <been_here.h>
#include <mail_params.h>
#include <mbox_conf.h>
#include <mbox_open.h>

/* Application-specific. */

#include "local.h"

#define STR	vstring_str

/* deliver_file - deliver to file */

int     deliver_file(LOCAL_STATE state, USER_ATTR usr_attr, char *path)
{
    char   *myname = "deliver_file";
    struct stat st;
    MBOX   *mp;
    VSTRING *why;
    int     mail_copy_status = MAIL_COPY_STAT_WRITE;
    int     deliver_status;
    int     copy_flags;

    /*
     * Make verbose logging easier to understand.
     */
    state.level++;
    if (msg_verbose)
	MSG_LOG_STATE(myname, state);

    /*
     * DUPLICATE ELIMINATION
     * 
     * Skip this file if it was already delivered to as this user.
     */
    if (been_here(state.dup_filter, "file %ld %s", (long) usr_attr.uid, path))
	return (0);

    /*
     * DELIVERY POLICY
     * 
     * Do we allow delivery to files?
     */
    if ((local_file_deliver_mask & state.msg_attr.exp_type) == 0)
	return (bounce_append(BOUNCE_FLAGS(state.request),
			      BOUNCE_ATTR(state.msg_attr),
			      "mail to file is restricted"));

    /*
     * Don't deliver trace-only requests.
     */
    if (DEL_REQ_TRACE_ONLY(state.request->flags))
	return (sent(BOUNCE_FLAGS(state.request), SENT_ATTR(state.msg_attr),
		     "delivers to file: %s", path));

    /*
     * DELIVERY RIGHTS
     * 
     * Use a default uid/gid when none are given.
     */
    if (usr_attr.uid == 0 && (usr_attr.uid = var_default_uid) == 0)
	msg_panic("privileged default user id");
    if (usr_attr.gid == 0 && (usr_attr.gid = var_default_gid) == 0)
	msg_panic("privileged default group id");

    /*
     * If the name ends in /, use maildir-style delivery instead.
     */
    if (path[strlen(path) - 1] == '/')
	return (deliver_maildir(state, usr_attr, path));

    /*
     * Deliver. From here on, no early returns or we have a memory leak.
     */
    if (msg_verbose)
	msg_info("deliver_file (%ld,%ld): %s",
		 (long) usr_attr.uid, (long) usr_attr.gid, path);
    if (vstream_fseek(state.msg_attr.fp, state.msg_attr.offset, SEEK_SET) < 0)
	msg_fatal("seek queue file %s: %m", state.msg_attr.queue_id);
    why = vstring_alloc(100);

    /*
     * As the specified user, open or create the file, lock it, and append
     * the message.
     */
    copy_flags = MAIL_COPY_MBOX;
    if ((local_deliver_hdr_mask & DELIVER_HDR_FILE) == 0)
	copy_flags &= ~MAIL_COPY_DELIVERED;

    set_eugid(usr_attr.uid, usr_attr.gid);
    mp = mbox_open(path, O_APPEND | O_CREAT | O_WRONLY,
		   S_IRUSR | S_IWUSR, &st, -1, -1,
		   local_mbox_lock_mask | MBOX_DOT_LOCK_MAY_FAIL, why);
    if (mp != 0) {
	if (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
	    vstream_fclose(mp->fp);
	    vstring_sprintf(why, "destination file is executable");
	    errno = 0;
	} else {
	    mail_copy_status = mail_copy(COPY_ATTR(state.msg_attr), mp->fp,
					 S_ISREG(st.st_mode) ? copy_flags :
					 (copy_flags & ~MAIL_COPY_TOFILE),
					 "\n", why);
	}
	mbox_release(mp);
    }
    set_eugid(var_owner_uid, var_owner_gid);

    /*
     * As the mail system, bounce, defer delivery, or report success.
     */
    if (mail_copy_status & MAIL_COPY_STAT_CORRUPT) {
	deliver_status = DEL_STAT_DEFER;
    } else if (mail_copy_status != 0) {
	deliver_status = (errno == EAGAIN || errno == ENOSPC || errno == ESTALE ?
			  defer_append : bounce_append)
	    (BOUNCE_FLAGS(state.request), BOUNCE_ATTR(state.msg_attr),
	     "cannot append message to destination file %s: %s",
	     path, STR(why));
    } else {
	deliver_status = sent(BOUNCE_FLAGS(state.request),
			      SENT_ATTR(state.msg_attr),
			      "delivered to file: %s", path);
    }

    /*
     * Clean up.
     */
    vstring_free(why);
    return (deliver_status);
}
