/*	$NetBSD: ptyfs_vnops.c,v 1.7 2005/02/26 22:58:55 perry Exp $	*/

/*
 * Copyright (c) 1993, 1995
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Jan-Simon Pendry.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)procfs_vnops.c	8.18 (Berkeley) 5/21/95
 */

/*
 * Copyright (c) 1993 Jan-Simon Pendry
 *
 * This code is derived from software contributed to Berkeley by
 * Jan-Simon Pendry.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)procfs_vnops.c	8.18 (Berkeley) 5/21/95
 */

/*
 * ptyfs vnode interface
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: ptyfs_vnops.c,v 1.7 2005/02/26 22:58:55 perry Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <sys/file.h>
#include <sys/filedesc.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/namei.h>
#include <sys/malloc.h>
#include <sys/mount.h>
#include <sys/select.h>
#include <sys/dirent.h>
#include <sys/resourcevar.h>
#include <sys/stat.h>
#include <sys/conf.h>
#include <sys/tty.h>
#include <sys/pty.h>

#include <uvm/uvm_extern.h>	/* for PAGE_SIZE */

#include <machine/reg.h>

#include <fs/ptyfs/ptyfs.h>
#include <miscfs/genfs/genfs.h>
#include <miscfs/specfs/specdev.h>

/*
 * Vnode Operations.
 *
 */

int	ptyfs_lookup	(void *);
#define	ptyfs_create	genfs_eopnotsupp
#define	ptyfs_mknod	genfs_eopnotsupp
int	ptyfs_open	(void *);
int	ptyfs_close	(void *);
int	ptyfs_access	(void *);
int	ptyfs_getattr	(void *);
int	ptyfs_setattr	(void *);
int	ptyfs_read	(void *);
int	ptyfs_write	(void *);
#define	ptyfs_fcntl	genfs_fcntl
int	ptyfs_ioctl	(void *);
int	ptyfs_poll	(void *);
int	ptyfs_kqfilter	(void *);
#define ptyfs_revoke	genfs_revoke
#define	ptyfs_mmap	genfs_eopnotsupp
#define	ptyfs_fsync	genfs_nullop
#define	ptyfs_seek	genfs_nullop
#define	ptyfs_remove	genfs_eopnotsupp
#define	ptyfs_link	genfs_abortop
#define	ptyfs_rename	genfs_eopnotsupp
#define	ptyfs_mkdir	genfs_eopnotsupp
#define	ptyfs_rmdir	genfs_eopnotsupp
#define	ptyfs_symlink	genfs_abortop
int	ptyfs_readdir	(void *);
#define	ptyfs_readlink	genfs_eopnotsupp
#define	ptyfs_abortop	genfs_abortop
int	ptyfs_reclaim	(void *);
#define	ptyfs_lock	genfs_lock
#define	ptyfs_unlock	genfs_unlock
#define	ptyfs_bmap	genfs_badop
#define	ptyfs_strategy	genfs_badop
int	ptyfs_print	(void *);
int	ptyfs_pathconf	(void *);
#define	ptyfs_islocked	genfs_islocked
#define	ptyfs_advlock	genfs_einval
#define	ptyfs_blkatoff	genfs_eopnotsupp
#define	ptyfs_valloc	genfs_eopnotsupp
#define	ptyfs_vfree	genfs_nullop
#define	ptyfs_truncate	genfs_eopnotsupp
int	ptyfs_update	(void *);
#define	ptyfs_bwrite	genfs_eopnotsupp
#define ptyfs_putpages	genfs_null_putpages

static int ptyfs_chown(struct vnode *, uid_t, gid_t, struct ucred *,
    struct proc *);
static int ptyfs_chmod(struct vnode *, mode_t, struct ucred *, struct proc *);
static void ptyfs_time(struct ptyfsnode *, struct timespec *,
    struct timespec *);
static int atoi(const char *, size_t);

extern const struct cdevsw pts_cdevsw, ptc_cdevsw;

/*
 * ptyfs vnode operations.
 */
int (**ptyfs_vnodeop_p)(void *);
const struct vnodeopv_entry_desc ptyfs_vnodeop_entries[] = {
	{ &vop_default_desc, vn_default_error },
	{ &vop_lookup_desc, ptyfs_lookup },		/* lookup */
	{ &vop_create_desc, ptyfs_create },		/* create */
	{ &vop_mknod_desc, ptyfs_mknod },		/* mknod */
	{ &vop_open_desc, ptyfs_open },			/* open */
	{ &vop_close_desc, ptyfs_close },		/* close */
	{ &vop_access_desc, ptyfs_access },		/* access */
	{ &vop_getattr_desc, ptyfs_getattr },		/* getattr */
	{ &vop_setattr_desc, ptyfs_setattr },		/* setattr */
	{ &vop_read_desc, ptyfs_read },			/* read */
	{ &vop_write_desc, ptyfs_write },		/* write */
	{ &vop_ioctl_desc, ptyfs_ioctl },		/* ioctl */
	{ &vop_fcntl_desc, ptyfs_fcntl },		/* fcntl */
	{ &vop_poll_desc, ptyfs_poll },			/* poll */
	{ &vop_kqfilter_desc, ptyfs_kqfilter },		/* kqfilter */
	{ &vop_revoke_desc, ptyfs_revoke },		/* revoke */
	{ &vop_mmap_desc, ptyfs_mmap },			/* mmap */
	{ &vop_fsync_desc, ptyfs_fsync },		/* fsync */
	{ &vop_seek_desc, ptyfs_seek },			/* seek */
	{ &vop_remove_desc, ptyfs_remove },		/* remove */
	{ &vop_link_desc, ptyfs_link },			/* link */
	{ &vop_rename_desc, ptyfs_rename },		/* rename */
	{ &vop_mkdir_desc, ptyfs_mkdir },		/* mkdir */
	{ &vop_rmdir_desc, ptyfs_rmdir },		/* rmdir */
	{ &vop_symlink_desc, ptyfs_symlink },		/* symlink */
	{ &vop_readdir_desc, ptyfs_readdir },		/* readdir */
	{ &vop_readlink_desc, ptyfs_readlink },		/* readlink */
	{ &vop_abortop_desc, ptyfs_abortop },		/* abortop */
	{ &vop_inactive_desc, spec_inactive },		/* inactive */
	{ &vop_reclaim_desc, ptyfs_reclaim },		/* reclaim */
	{ &vop_lock_desc, ptyfs_lock },			/* lock */
	{ &vop_unlock_desc, ptyfs_unlock },		/* unlock */
	{ &vop_bmap_desc, ptyfs_bmap },			/* bmap */
	{ &vop_strategy_desc, ptyfs_strategy },		/* strategy */
	{ &vop_print_desc, ptyfs_print },		/* print */
	{ &vop_islocked_desc, ptyfs_islocked },		/* islocked */
	{ &vop_pathconf_desc, ptyfs_pathconf },		/* pathconf */
	{ &vop_advlock_desc, ptyfs_advlock },		/* advlock */
	{ &vop_blkatoff_desc, ptyfs_blkatoff },		/* blkatoff */
	{ &vop_valloc_desc, ptyfs_valloc },		/* valloc */
	{ &vop_vfree_desc, ptyfs_vfree },		/* vfree */
	{ &vop_truncate_desc, ptyfs_truncate },		/* truncate */
	{ &vop_update_desc, ptyfs_update },		/* update */
	{ &vop_bwrite_desc, ptyfs_bwrite },		/* bwrite */
	{ &vop_putpages_desc, ptyfs_putpages },		/* putpages */
	{ NULL, NULL }
};
const struct vnodeopv_desc ptyfs_vnodeop_opv_desc =
	{ &ptyfs_vnodeop_p, ptyfs_vnodeop_entries };

/*
 * _reclaim is called when getnewvnode()
 * wants to make use of an entry on the vnode
 * free list.  at this time the filesystem needs
 * to free any private data and remove the node
 * from any private lists.
 */
int
ptyfs_reclaim(void *v)
{
	struct vop_reclaim_args /* {
		struct vnode *a_vp;
	} */ *ap = v;
	return ptyfs_freevp(ap->a_vp);
}

/*
 * Return POSIX pathconf information applicable to special devices.
 */
int
ptyfs_pathconf(void *v)
{
	struct vop_pathconf_args /* {
		struct vnode *a_vp;
		int a_name;
		register_t *a_retval;
	} */ *ap = v;

	switch (ap->a_name) {
	case _PC_LINK_MAX:
		*ap->a_retval = LINK_MAX;
		return 0;
	case _PC_MAX_CANON:
		*ap->a_retval = MAX_CANON;
		return 0;
	case _PC_MAX_INPUT:
		*ap->a_retval = MAX_INPUT;
		return 0;
	case _PC_PIPE_BUF:
		*ap->a_retval = PIPE_BUF;
		return 0;
	case _PC_CHOWN_RESTRICTED:
		*ap->a_retval = 1;
		return 0;
	case _PC_VDISABLE:
		*ap->a_retval = _POSIX_VDISABLE;
		return 0;
	case _PC_SYNC_IO:
		*ap->a_retval = 1;
		return 0;
	default:
		return EINVAL;
	}
}

/*
 * _print is used for debugging.
 * just print a readable description
 * of (vp).
 */
int
ptyfs_print(void *v)
{
	struct vop_print_args /* {
		struct vnode *a_vp;
	} */ *ap = v;
	struct ptyfsnode *ptyfs = VTOPTYFS(ap->a_vp);

	printf("tag VT_PTYFS, type %d, pty %d\n",
	    ptyfs->ptyfs_type, ptyfs->ptyfs_pty);
	return 0;
}

/*
 * Invent attributes for ptyfsnode (vp) and store
 * them in (vap).
 * Directories lengths are returned as zero since
 * any real length would require the genuine size
 * to be computed, and nothing cares anyway.
 *
 * this is relatively minimal for ptyfs.
 */
int
ptyfs_getattr(void *v)
{
	struct vop_getattr_args /* {
		struct vnode *a_vp;
		struct vattr *a_vap;
		struct ucred *a_cred;
		struct proc *a_p;
	} */ *ap = v;
	struct ptyfsnode *ptyfs = VTOPTYFS(ap->a_vp);
	struct vattr *vap = ap->a_vap;
        struct timespec ts;

	TIMEVAL_TO_TIMESPEC(&time, &ts);
	ptyfs_time(ptyfs, &ts, &ts);

	/* start by zeroing out the attributes */
	VATTR_NULL(vap);

	/* next do all the common fields */
	vap->va_type = ap->a_vp->v_type;
	vap->va_fsid = ap->a_vp->v_mount->mnt_stat.f_fsidx.__fsid_val[0];
	vap->va_fileid = ptyfs->ptyfs_fileno;
	vap->va_gen = 0;
	vap->va_flags = 0;
	vap->va_nlink = 1;
	vap->va_blocksize = PAGE_SIZE;

	vap->va_atime = ptyfs->ptyfs_atime;
	vap->va_mtime = ptyfs->ptyfs_mtime;
	vap->va_ctime = ptyfs->ptyfs_ctime;
	vap->va_birthtime = ptyfs->ptyfs_birthtime;
	vap->va_mode = ptyfs->ptyfs_mode;
	vap->va_flags = ptyfs->ptyfs_flags;
	vap->va_uid = ptyfs->ptyfs_uid;
	vap->va_gid = ptyfs->ptyfs_gid;

	switch (ptyfs->ptyfs_type) {
	case PTYFSpts:
	case PTYFSptc:
		if (pty_isfree(ptyfs->ptyfs_pty, 1))
			return ENOENT;
		vap->va_bytes = vap->va_size = 0;
		vap->va_rdev = ap->a_vp->v_rdev;
		break;
	case PTYFSroot:
		vap->va_rdev = 0;
		vap->va_bytes = vap->va_size = DEV_BSIZE;
		break;

	default:
		return EOPNOTSUPP;
	}

	return 0;
}

/*ARGSUSED*/
int
ptyfs_setattr(void *v)
{
	struct vop_setattr_args /* {
		struct vnodeop_desc *a_desc;
		struct vnode *a_vp;
		struct vattr *a_vap;
		struct ucred *a_cred;
		struct proc *a_p;
	} */ *ap = v;
	struct vnode *vp = ap->a_vp;
	struct ptyfsnode *ptyfs = VTOPTYFS(vp);
	struct vattr *vap = ap->a_vap;
	struct ucred *cred = ap->a_cred;
	struct proc *p = ap->a_p;
	int error;

	if (vap->va_size != VNOVAL) {
 		switch (ptyfs->ptyfs_type) {
 		case PTYFSroot:
 			return EISDIR;
 		case PTYFSpts:
 		case PTYFSptc:
			break;
		default:
			return EINVAL;
		}
	}

	if (vap->va_flags != VNOVAL) {
		if (vp->v_mount->mnt_flag & MNT_RDONLY)
			return EROFS;
		if (cred->cr_uid != ptyfs->ptyfs_uid &&
		    (error = suser(cred, &p->p_acflag)) != 0)
			return error;
		if (cred->cr_uid == 0) {
			if ((ptyfs->ptyfs_flags & (SF_IMMUTABLE | SF_APPEND)) &&
			    securelevel > 0)
				return EPERM;
			/* Snapshot flag cannot be set or cleared */
			if ((vap->va_flags & SF_SNAPSHOT) !=
			    (ptyfs->ptyfs_flags & SF_SNAPSHOT))
				return EPERM;
			ptyfs->ptyfs_flags = vap->va_flags;
		} else {
			if ((ptyfs->ptyfs_flags & (SF_IMMUTABLE | SF_APPEND)) ||
			    (vap->va_flags & UF_SETTABLE) != vap->va_flags)
				return EPERM;
			if ((ptyfs->ptyfs_flags & SF_SETTABLE) !=
			    (vap->va_flags & SF_SETTABLE))
				return EPERM;
			ptyfs->ptyfs_flags &= SF_SETTABLE;
			ptyfs->ptyfs_flags |= (vap->va_flags & UF_SETTABLE);
		}
		ptyfs->ptyfs_flag |= PTYFS_CHANGE;
		if (vap->va_flags & (IMMUTABLE | APPEND))
			return 0;
	}
	if (ptyfs->ptyfs_flags & (IMMUTABLE | APPEND))
		return EPERM;
	/*
	 * Go through the fields and update iff not VNOVAL.
	 */
	if (vap->va_uid != (uid_t)VNOVAL || vap->va_gid != (gid_t)VNOVAL) {
		if (vp->v_mount->mnt_flag & MNT_RDONLY)
			return EROFS;
		if (ptyfs->ptyfs_type == PTYFSroot)
			return EPERM;
		error = ptyfs_chown(vp, vap->va_uid, vap->va_gid, cred, p);
		if (error)
			return error;
	}

	if (vap->va_atime.tv_sec != VNOVAL || vap->va_mtime.tv_sec != VNOVAL ||
	    vap->va_birthtime.tv_sec != VNOVAL) {
		if (vp->v_mount->mnt_flag & MNT_RDONLY)
			return EROFS;
		if ((ptyfs->ptyfs_flags & SF_SNAPSHOT) != 0)
			return EPERM;
		if (cred->cr_uid != ptyfs->ptyfs_uid &&
		    (error = suser(cred, &p->p_acflag)) &&
		    ((vap->va_vaflags & VA_UTIMES_NULL) == 0 ||
		    (error = VOP_ACCESS(vp, VWRITE, cred, p)) != 0))
			return (error);
		if (vap->va_atime.tv_sec != VNOVAL)
			if (!(vp->v_mount->mnt_flag & MNT_NOATIME))
				ptyfs->ptyfs_flag |= PTYFS_ACCESS;
		if (vap->va_mtime.tv_sec != VNOVAL)
			ptyfs->ptyfs_flag |= PTYFS_CHANGE | PTYFS_MODIFY;
		if (vap->va_birthtime.tv_sec != VNOVAL)
			ptyfs->ptyfs_birthtime = vap->va_birthtime;
		ptyfs->ptyfs_flag |= PTYFS_CHANGE;
		error = VOP_UPDATE(vp, &vap->va_atime, &vap->va_mtime, 0);
		if (error)
			return error;
	}
	if (vap->va_mode != (mode_t)VNOVAL) {
		if (vp->v_mount->mnt_flag & MNT_RDONLY)
			return EROFS;
		if (ptyfs->ptyfs_type == PTYFSroot)
			return EPERM;
		if ((ptyfs->ptyfs_flags & SF_SNAPSHOT) != 0 &&
		    (vap->va_mode &
		    (S_IXUSR|S_IWUSR|S_IXGRP|S_IWGRP|S_IXOTH|S_IWOTH)))
			return EPERM;
		error = ptyfs_chmod(vp, vap->va_mode, cred, p);
		if (error)
			return error;
	}
	VN_KNOTE(vp, NOTE_ATTRIB);
	return 0;
}

/*
 * Change the mode on a file.
 * Inode must be locked before calling.
 */
static int
ptyfs_chmod(struct vnode *vp, mode_t mode, struct ucred *cred, struct proc *p)
{
	struct ptyfsnode *ptyfs = VTOPTYFS(vp);
	int error;

	if (cred->cr_uid != ptyfs->ptyfs_uid &&
	    (error = suser(cred, &p->p_acflag)) != 0)
		return error;
	ptyfs->ptyfs_mode &= ~ALLPERMS;
	ptyfs->ptyfs_mode |= (mode & ALLPERMS);
	return 0;
}

/*
 * Perform chown operation on inode ip;
 * inode must be locked prior to call.
 */
static int
ptyfs_chown(struct vnode *vp, uid_t uid, gid_t gid, struct ucred *cred,
    struct proc *p)
{
	struct ptyfsnode *ptyfs = VTOPTYFS(vp);
	int		error;

	if (uid == (uid_t)VNOVAL)
		uid = ptyfs->ptyfs_uid;
	if (gid == (gid_t)VNOVAL)
		gid = ptyfs->ptyfs_gid;
	/*
	 * If we don't own the file, are trying to change the owner
	 * of the file, or are not a member of the target group,
	 * the caller's credentials must imply super-user privilege
	 * or the call fails.
	 */
	if ((cred->cr_uid != ptyfs->ptyfs_uid || uid != ptyfs->ptyfs_uid ||
	    (gid != ptyfs->ptyfs_gid &&
	     !(cred->cr_gid == gid || groupmember((gid_t)gid, cred)))) &&
	    ((error = suser(cred, &p->p_acflag)) != 0))
		return error;

	ptyfs->ptyfs_gid = gid;
	ptyfs->ptyfs_uid = uid;
	return 0;
}

/*
 * implement access checking.
 *
 * actually, the check for super-user is slightly
 * broken since it will allow read access to write-only
 * objects.  this doesn't cause any particular trouble
 * but does mean that the i/o entry points need to check
 * that the operation really does make sense.
 */
int
ptyfs_access(void *v)
{
	struct vop_access_args /* {
		struct vnode *a_vp;
		int a_mode;
		struct ucred *a_cred;
		struct proc *a_p;
	} */ *ap = v;
	struct vattr va;
	int error;

	if ((error = VOP_GETATTR(ap->a_vp, &va, ap->a_cred, ap->a_p)) != 0)
		return error;

	return vaccess(va.va_type, va.va_mode,
	    va.va_uid, va.va_gid, ap->a_mode, ap->a_cred);
}

/*
 * lookup.  this is incredibly complicated in the
 * general case, however for most pseudo-filesystems
 * very little needs to be done.
 *
 * Locking isn't hard here, just poorly documented.
 *
 * If we're looking up ".", just vref the parent & return it.
 *
 * If we're looking up "..", unlock the parent, and lock "..". If everything
 * went ok, and we're on the last component and the caller requested the
 * parent locked, try to re-lock the parent. We do this to prevent lock
 * races.
 *
 * For anything else, get the needed node. Then unlock the parent if not
 * the last component or not LOCKPARENT (i.e. if we wouldn't re-lock the
 * parent in the .. case).
 *
 * We try to exit with the parent locked in error cases.
 */
int
ptyfs_lookup(void *v)
{
	struct vop_lookup_args /* {
		struct vnode * a_dvp;
		struct vnode ** a_vpp;
		struct componentname * a_cnp;
	} */ *ap = v;
	struct componentname *cnp = ap->a_cnp;
	struct vnode **vpp = ap->a_vpp;
	struct vnode *dvp = ap->a_dvp;
	const char *pname = cnp->cn_nameptr;
	struct ptyfsnode *ptyfs;
	int pty, error, wantpunlock;

	*vpp = NULL;
	cnp->cn_flags &= ~PDIRUNLOCK;

	if (cnp->cn_nameiop == DELETE || cnp->cn_nameiop == RENAME)
		return EROFS;

	if (cnp->cn_namelen == 1 && *pname == '.') {
		*vpp = dvp;
		VREF(dvp);
		return 0;
	}

	wantpunlock = ~cnp->cn_flags & (LOCKPARENT | ISLASTCN);
	ptyfs = VTOPTYFS(dvp);
	switch (ptyfs->ptyfs_type) {
	case PTYFSroot:
		/*
		 * Shouldn't get here with .. in the root node.
		 */
		if (cnp->cn_flags & ISDOTDOT)
			return EIO;

		pty = atoi(pname, cnp->cn_namelen);

		if (pty < 0 || pty >= npty || pty_isfree(pty, 1))
			break;

		error = ptyfs_allocvp(dvp->v_mount, vpp, PTYFSpts, pty,
		    curproc);
		if (error == 0 && wantpunlock) {
			VOP_UNLOCK(dvp, 0);
			cnp->cn_flags |= PDIRUNLOCK;
		}
		return error;

	default:
		return ENOTDIR;
	}

	return cnp->cn_nameiop == LOOKUP ? ENOENT : EROFS;
}

/*
 * readdir returns directory entries from ptyfsnode (vp).
 *
 * the strategy here with ptyfs is to generate a single
 * directory entry at a time (struct dirent) and then
 * copy that out to userland using uiomove.  a more efficent
 * though more complex implementation, would try to minimize
 * the number of calls to uiomove().  for ptyfs, this is
 * hardly worth the added code complexity.
 *
 * this should just be done through read()
 */
int
ptyfs_readdir(void *v)
{
	struct vop_readdir_args /* {
		struct vnode *a_vp;
		struct uio *a_uio;
		struct ucred *a_cred;
		int *a_eofflag;
		off_t **a_cookies;
		int *a_ncookies;
	} */ *ap = v;
	struct uio *uio = ap->a_uio;
	struct dirent d;
	struct ptyfsnode *ptyfs;
	off_t i;
	int error;
	off_t *cookies = NULL;
	int ncookies;
	struct vnode *vp;
	int nc = 0;

	vp = ap->a_vp;
	ptyfs = VTOPTYFS(vp);

	if (uio->uio_resid < UIO_MX)
		return EINVAL;
	if (uio->uio_offset < 0)
		return EINVAL;

	error = 0;
	i = uio->uio_offset;
	(void)memset(&d, 0, sizeof(d));
	d.d_reclen = UIO_MX;
	ncookies = uio->uio_resid / UIO_MX;

	switch (ptyfs->ptyfs_type) {
	case PTYFSroot: /* root */

		if (i >= npty)
			return 0;

		if (ap->a_ncookies) {
			ncookies = min(ncookies, (npty + 2 - i));
			cookies = malloc(ncookies * sizeof (off_t),
			    M_TEMP, M_WAITOK);
			*ap->a_cookies = cookies;
		}

		for (; i < 2; i++) {
			switch (i) {
			case 0:		/* `.' */
			case 1:		/* `..' */
				d.d_fileno = PTYFS_FILENO(0, PTYFSroot);
				d.d_namlen = i + 1;
				(void)memcpy(d.d_name, "..", d.d_namlen);
				d.d_name[i + 1] = '\0';
				d.d_type = DT_DIR;
				break;
			}
			if ((error = uiomove(&d, UIO_MX, uio)) != 0)
				break;
			if (cookies)
				*cookies++ = i + 1;
			nc++;
		}
		if (error) {
			ncookies = nc;
			break;
		}
		for (; uio->uio_resid >= UIO_MX && i < npty; i++) {
			/* check for used ptys */
			if (pty_isfree(i - 2, 1))
				continue;

			d.d_fileno = PTYFS_FILENO(i - 2, PTYFSpts);
			d.d_namlen = snprintf(d.d_name, sizeof(d.d_name),
			    "%lld", (long long)(i - 2));
			d.d_type = DT_CHR;
			if ((error = uiomove(&d, UIO_MX, uio)) != 0)
				break;
			if (cookies)
				*cookies++ = i + 1;
			nc++;
		}
		ncookies = nc;
		break;

	default:
		error = ENOTDIR;
		break;
	}

	if (ap->a_ncookies) {
		if (error) {
			if (cookies)
				free(*ap->a_cookies, M_TEMP);
			*ap->a_ncookies = 0;
			*ap->a_cookies = NULL;
		} else
			*ap->a_ncookies = ncookies;
	}
	uio->uio_offset = i;
	return error;
}

int
ptyfs_open(void *v)
{
	struct vop_open_args /* {
		struct vnode *a_vp;
		int  a_mode;
		struct ucred *a_cred;
		struct proc *a_p;
	} */ *ap = v;
	struct vnode *vp = ap->a_vp;
	struct ptyfsnode *ptyfs = VTOPTYFS(vp);

	ptyfs->ptyfs_flag |= PTYFS_CHANGE|PTYFS_ACCESS;
	switch (ptyfs->ptyfs_type) {
	case PTYFSpts:
	case PTYFSptc:
		return spec_open(v);
	case PTYFSroot:
		return 0;
	default:
		return EINVAL;
	}
}

int
ptyfs_close(void *v)
{
	struct vop_close_args /* {
		struct vnode *a_vp;
		int  a_fflag;
		struct ucred *a_cred;
		struct proc *a_p;
	} */ *ap = v;
	struct vnode *vp = ap->a_vp;
	struct ptyfsnode *ptyfs = VTOPTYFS(vp);
        struct timespec ts;

        simple_lock(&vp->v_interlock);
        if (vp->v_usecount > 1) {
		TIMEVAL_TO_TIMESPEC(&time, &ts);
		ptyfs_time(ptyfs, &ts, &ts);
        }
        simple_unlock(&vp->v_interlock);

	switch (ptyfs->ptyfs_type) {
	case PTYFSpts:
	case PTYFSptc:
		return spec_close(v);
	case PTYFSroot:
		return 0;
	default:
		return EINVAL;
	}
}

int
ptyfs_read(void *v)
{
	struct vop_read_args /* {
		struct vnode *a_vp;
		struct uio *a_uio;
		int  a_ioflag;
		struct ucred *a_cred;
	} */ *ap = v;
	struct vnode *vp = ap->a_vp;
	struct ptyfsnode *ptyfs = VTOPTYFS(vp);
	int error;

	ptyfs->ptyfs_flag |= PTYFS_ACCESS;
	switch (ptyfs->ptyfs_type) {
	case PTYFSpts:
		VOP_UNLOCK(vp, 0);
		error = (*pts_cdevsw.d_read)(vp->v_rdev, ap->a_uio,
		    ap->a_ioflag);
		vn_lock(vp, LK_RETRY|LK_EXCLUSIVE);
		return error;
	case PTYFSptc:
		VOP_UNLOCK(vp, 0);
		error = (*ptc_cdevsw.d_read)(vp->v_rdev, ap->a_uio,
		    ap->a_ioflag);
		vn_lock(vp, LK_RETRY|LK_EXCLUSIVE);
		return error;
	default:
		return EOPNOTSUPP;
	}
}

int
ptyfs_write(void *v)
{
	struct vop_write_args /* {
		struct vnode *a_vp;
		struct uio *a_uio;
		int  a_ioflag;
		struct ucred *a_cred;
	} */ *ap = v;
	int error;
	struct vnode *vp = ap->a_vp;
	struct ptyfsnode *ptyfs = VTOPTYFS(vp);

	ptyfs->ptyfs_flag |= PTYFS_MODIFY;
	switch (ptyfs->ptyfs_type) {
	case PTYFSpts:
		VOP_UNLOCK(vp, 0);
		error = (*pts_cdevsw.d_write)(vp->v_rdev, ap->a_uio,
		    ap->a_ioflag);
		vn_lock(vp, LK_RETRY|LK_EXCLUSIVE);
		return error;
	case PTYFSptc:
		VOP_UNLOCK(vp, 0);
		error = (*ptc_cdevsw.d_write)(vp->v_rdev, ap->a_uio,
		    ap->a_ioflag);
		vn_lock(vp, LK_RETRY|LK_EXCLUSIVE);
		return error;
	default:
		return EOPNOTSUPP;
	}
}

int
ptyfs_ioctl(void *v)
{
	struct vop_ioctl_args /* {
		struct vnode *a_vp;
		u_long a_command;
		void *a_data;
		int  a_fflag;
		struct ucred *a_cred;
		struct proc *a_p;
	} */ *ap = v;
	struct vnode *vp = ap->a_vp;
	struct ptyfsnode *ptyfs = VTOPTYFS(vp);

	switch (ptyfs->ptyfs_type) {
	case PTYFSpts:
		return (*pts_cdevsw.d_ioctl)(vp->v_rdev, ap->a_command,
		    ap->a_data, ap->a_fflag, ap->a_p);
	case PTYFSptc:
		return (*ptc_cdevsw.d_ioctl)(vp->v_rdev, ap->a_command,
		    ap->a_data, ap->a_fflag, ap->a_p);
	default:
		return EOPNOTSUPP;
	}
}

int
ptyfs_poll(void *v)
{
	struct vop_poll_args /* {
		struct vnode *a_vp;
		int a_events;
		struct proc *a_p;
	} */ *ap = v;
	struct vnode *vp = ap->a_vp;
	struct ptyfsnode *ptyfs = VTOPTYFS(vp);

	switch (ptyfs->ptyfs_type) {
	case PTYFSpts:
		return (*pts_cdevsw.d_poll)(vp->v_rdev, ap->a_events, ap->a_p);
	case PTYFSptc:
		return (*ptc_cdevsw.d_poll)(vp->v_rdev, ap->a_events, ap->a_p);
	default:
		return genfs_poll(v);
	}
}

int
ptyfs_kqfilter(void *v)
{
	struct vop_kqfilter_args /* {
		struct vnode *a_vp;
		struct knote *a_kn;
	} */ *ap = v;
	struct vnode *vp = ap->a_vp;
	struct ptyfsnode *ptyfs = VTOPTYFS(vp);

	switch (ptyfs->ptyfs_type) {
	case PTYFSpts:
		return (*pts_cdevsw.d_kqfilter)(vp->v_rdev, ap->a_kn);
	case PTYFSptc:
		return (*ptc_cdevsw.d_kqfilter)(vp->v_rdev, ap->a_kn);
	default:
		return genfs_kqfilter(v);
	}
}

int
ptyfs_update(v)
	void *v;
{
	struct vop_update_args /* {
		struct vnode *a_vp;
		struct timespec *a_access;
		struct timespec *a_modify;
		int a_flags;
	} */ *ap = v;
	struct ptyfsnode *ptyfs = VTOPTYFS(ap->a_vp);
	struct timespec ts;

	if (ap->a_vp->v_mount->mnt_flag & MNT_RDONLY)
		return 0;

	TIMEVAL_TO_TIMESPEC(&time, &ts);
	if (ap->a_access == NULL)
		ap->a_access = &ts;
	if (ap->a_modify == NULL)
		ap->a_modify = &ts;
	ptyfs_time(ptyfs, ap->a_access, ap->a_modify);
	return 0;
}

static void
ptyfs_time(struct ptyfsnode *ptyfs, struct timespec *atime,
    struct timespec *mtime)
{
	if (ptyfs->ptyfs_flag & PTYFS_MODIFY) {
		ptyfs->ptyfs_mtime = *mtime;
		ptyfs->ptyfs_atime = *atime;
	} else if (ptyfs->ptyfs_flag & PTYFS_ACCESS)
		ptyfs->ptyfs_atime = *atime;
	if (ptyfs->ptyfs_flag & PTYFS_CHANGE)
		ptyfs->ptyfs_ctime = *atime;
	ptyfs->ptyfs_flag = 0;
}

/*
 * convert decimal ascii to int
 */
static int
atoi(const char *b, size_t len)
{
	int p = 0;

	while (len--) {
		char c = *b++;
		if (c < '0' || c > '9')
			return -1;
		p = 10 * p + (c - '0');
	}

	return p;
}
