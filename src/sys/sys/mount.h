/*	$NetBSD: mount.h,v 1.127 2005/02/03 19:20:01 perry Exp $	*/

/*
 * Copyright (c) 1989, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *	@(#)mount.h	8.21 (Berkeley) 5/20/95
 */

#ifndef _SYS_MOUNT_H_
#define _SYS_MOUNT_H_

#ifdef _KERNEL_OPT
#include "opt_compat_43.h"
#endif

#ifndef _KERNEL
#include <sys/featuretest.h>
#include <sys/ucred.h>
#if defined(_NETBSD_SOURCE)
#include <sys/stat.h>
#endif /* _NETBSD_SOURCE */
#endif
#include <sys/fstypes.h>
#include <sys/queue.h>
#include <sys/lock.h>
#include <sys/statvfs.h>

/*
 * file system statistics
 */

#define	MFSNAMELEN	16	/* length of fs type name, including nul */
#define	MNAMELEN	90	/* length of buffer for returned name */

#if defined(__LIBC12_SOURCE__) || defined(_KERNEL)
struct statfs12 {
	short	f_type;			/* type of file system */
	u_short	f_oflags;		/* deprecated copy of mount flags */
	long	f_bsize;		/* fundamental file system block size */
	long	f_iosize;		/* optimal transfer block size */
	long	f_blocks;		/* total data blocks in file system */
	long	f_bfree;		/* free blocks in fs */
	long	f_bavail;		/* free blocks avail to non-superuser */
	long	f_files;		/* total file nodes in file system */
	long	f_ffree;		/* free file nodes in fs */
	fsid_t	f_fsid;			/* file system id */
	uid_t	f_owner;		/* user that mounted the file system */
	long	f_flags;		/* copy of mount flags */
	long	f_syncwrites;		/* count of sync writes since mount */
	long	f_asyncwrites;		/* count of async writes since mount */
	long	f_spare[1];		/* spare for later */
	char	f_fstypename[MFSNAMELEN]; /* fs type name */
	char	f_mntonname[MNAMELEN];	  /* directory on which mounted */
	char	f_mntfromname[MNAMELEN];  /* mounted file system */
};
#endif

/*
 * File system types.
 */
#define	MOUNT_FFS	"ffs"		/* UNIX "Fast" Filesystem */
#define	MOUNT_UFS	MOUNT_FFS	/* for compatibility */
#define	MOUNT_NFS	"nfs"		/* Network Filesystem */
#define	MOUNT_MFS	"mfs"		/* Memory Filesystem */
#define	MOUNT_MSDOS	"msdos"		/* MSDOS Filesystem */
#define	MOUNT_LFS	"lfs"		/* Log-based Filesystem */
#define	MOUNT_FDESC	"fdesc"		/* File Descriptor Filesystem */
#define	MOUNT_PORTAL	"portal"	/* Portal Filesystem */
#define	MOUNT_NULL	"null"		/* Minimal Filesystem Layer */
#define	MOUNT_OVERLAY	"overlay"	/* Minimal Overlay Filesystem Layer */
#define	MOUNT_UMAP	"umap"	/* User/Group Identifier Remapping Filesystem */
#define	MOUNT_KERNFS	"kernfs"	/* Kernel Information Filesystem */
#define	MOUNT_PROCFS	"procfs"	/* /proc Filesystem */
#define	MOUNT_AFS	"afs"		/* Andrew Filesystem */
#define	MOUNT_CD9660	"cd9660"	/* ISO9660 (aka CDROM) Filesystem */
#define	MOUNT_UNION	"union"		/* Union (translucent) Filesystem */
#define	MOUNT_ADOSFS	"adosfs"	/* AmigaDOS Filesystem */
#define	MOUNT_EXT2FS	"ext2fs"	/* Second Extended Filesystem */
#define	MOUNT_CFS	"coda"		/* Coda Filesystem */
#define	MOUNT_CODA	MOUNT_CFS	/* Coda Filesystem */
#define	MOUNT_FILECORE	"filecore"	/* Acorn Filecore Filesystem */
#define	MOUNT_NTFS	"ntfs"		/* Windows/NT Filesystem */
#define	MOUNT_SMBFS	"smbfs"		/* CIFS (SMB) */
#define	MOUNT_PTYFS	"ptyfs"		/* Pseudo tty filesystem */

/*
 * Structure per mounted file system.  Each mounted file system has an
 * array of operations and an instance record.  The file systems are
 * put on a doubly linked list.
 */
LIST_HEAD(vnodelst, vnode);

struct mount {
	CIRCLEQ_ENTRY(mount) mnt_list;		/* mount list */
	struct vfsops	*mnt_op;		/* operations on fs */
	struct vnode	*mnt_vnodecovered;	/* vnode we mounted on */
	struct vnode	*mnt_syncer;		/* syncer vnode */
	struct vnodelst	mnt_vnodelist;		/* list of vnodes this mount */
	struct lock	mnt_lock;		/* mount structure lock */
	int		mnt_flag;		/* flags */
	int		mnt_iflag;		/* internal flags */
	int		mnt_fs_bshift;		/* offset shift for lblkno */
	int		mnt_dev_bshift;		/* shift for device sectors */
	struct statvfs	mnt_stat;		/* cache of filesystem stats */
	void		*mnt_data;		/* private data */
	int		mnt_wcnt;		/* count of vfs_busy waiters */
	struct proc	*mnt_unmounter;		/* who is unmounting */
	int		mnt_writeopcountupper;	/* upper writeops in progress */
	int		mnt_writeopcountlower;	/* lower writeops in progress */
	struct simplelock mnt_slock;		/* mutex for wcnt and
						   writeops counters */
	struct mount	*mnt_leaf;		/* leaf fs we mounted on */
};

/*
 * Sysctl CTL_VFS definitions.
 *
 * Second level identifier specifies which filesystem. Second level
 * identifier VFS_GENERIC returns information about all filesystems.
 *
 * Note the slightly non-flat nature of these sysctl numbers.  Oh for
 * a better sysctl interface.
 */
#define VFS_GENERIC	0		/* generic filesystem information */
#define VFS_MAXTYPENUM	1		/* int: highest defined fs type */
#define VFS_CONF	2		/* struct: vfsconf for filesystem given
					   as next argument */
#define VFS_USERMOUNT	3		/* enable/disable fs mnt by non-root */
#define	VFSGEN_MAXID	4		/* number of valid vfs.generic ids */

/*
 * USE THE SAME NAMES AS MOUNT_*!
 *
 * Only need to add new entry here if the filesystem actually supports
 * sysctl(2).
 */
#define	CTL_VFS_NAMES { \
	{ "generic", CTLTYPE_NODE }, \
	{ MOUNT_FFS, CTLTYPE_NODE }, \
	{ MOUNT_NFS, CTLTYPE_NODE }, \
	{ MOUNT_MFS, CTLTYPE_NODE }, \
	{ MOUNT_MSDOS, CTLTYPE_NODE }, \
	{ MOUNT_LFS, CTLTYPE_NODE }, \
	{ 0, 0 }, 			/* MOUNT_LOFS */ \
	{ MOUNT_FDESC, CTLTYPE_NODE }, \
	{ MOUNT_PORTAL, CTLTYPE_NODE }, \
	{ MOUNT_NULL, CTLTYPE_NODE }, \
	{ MOUNT_UMAP, CTLTYPE_NODE }, \
	{ MOUNT_KERNFS, CTLTYPE_NODE }, \
	{ MOUNT_PROCFS, CTLTYPE_NODE }, \
	{ MOUNT_AFS, CTLTYPE_NODE }, \
	{ MOUNT_CD9660, CTLTYPE_NODE }, \
	{ MOUNT_UNION, CTLTYPE_NODE }, \
	{ MOUNT_ADOSFS, CTLTYPE_NODE }, \
	{ MOUNT_EXT2FS, CTLTYPE_NODE }, \
	{ MOUNT_CODA, CTLTYPE_NODE }, \
	{ MOUNT_FILECORE, CTLTYPE_NODE }, \
	{ MOUNT_NTFS, CTLTYPE_NODE }, \
}

#define	VFS_MAXID	20		/* number of valid vfs ids */

#define	CTL_VFSGENCTL_NAMES { \
	{ 0, 0 }, \
	{ "maxtypenum", CTLTYPE_INT }, \
	{ "conf", CTLTYPE_NODE }, 	/* Special */ \
	{ "usermount", CTLTYPE_INT }, \
}

/*
 * Operations supported on mounted file system.
 */
#ifdef _KERNEL

#if defined(COMPAT_09) || defined(COMPAT_43) || defined(COMPAT_44)

/*
 * Filesystem configuration information. Not used by NetBSD, but
 * defined here to provide a compatible sysctl interface to Lite2.
 */
struct vfsconf {
	struct	vfsops *vfc_vfsops;	/* filesystem operations vector */
	char	vfc_name[MFSNAMELEN]; 	/* filesystem type name */
	int	vfc_typenum;		/* historic filesystem type number */
	int  	vfc_refcount;		/* number mounted of this type */
	int	vfc_flags;		/* permanent flags */
	int	(*vfc_mountroot)(void);	/* if != NULL, routine to mount root */
	struct	vfsconf *vfc_next; 	/* next in list */
};

#endif

#if __STDC__
struct nameidata;
struct mbuf;
struct vnodeopv_desc;
#endif

struct vfsops {
	const char *vfs_name;
	int	(*vfs_mount)	(struct mount *, const char *, void *,
				    struct nameidata *, struct proc *);
	int	(*vfs_start)	(struct mount *, int, struct proc *);
	int	(*vfs_unmount)	(struct mount *, int, struct proc *);
	int	(*vfs_root)	(struct mount *, struct vnode **);
	int	(*vfs_quotactl)	(struct mount *, int, uid_t, void *,
				    struct proc *);
	int	(*vfs_statvfs)	(struct mount *, struct statvfs *,
				    struct proc *);
	int	(*vfs_sync)	(struct mount *, int, struct ucred *,
				    struct proc *);
	int	(*vfs_vget)	(struct mount *, ino_t, struct vnode **);
	int	(*vfs_fhtovp)	(struct mount *, struct fid *,
				    struct vnode **);
	int	(*vfs_vptofh)	(struct vnode *, struct fid *);
	void	(*vfs_init)	(void);
	void	(*vfs_reinit)	(void);
	void	(*vfs_done)	(void);
	int	*vfs_wassysctl;			/* @@@ no longer useful */
	int	(*vfs_mountroot)(void);
	int	(*vfs_checkexp) (struct mount *, struct mbuf *, int *,
				    struct ucred **);
	int	(*vfs_snapshot)	(struct mount *, struct vnode *,
				    struct timespec *);
	int	(*vfs_extattrctl) (struct mount *, int,
				    struct vnode *, int, const char *,
				    struct proc *);
	const struct vnodeopv_desc * const *vfs_opv_descs;
	int	vfs_refcount;
	LIST_ENTRY(vfsops) vfs_list;
};

#define VFS_MOUNT(MP, PATH, DATA, NDP, P) \
	(*(MP)->mnt_op->vfs_mount)(MP, PATH, DATA, NDP, P)
#define VFS_START(MP, FLAGS, P)	  (*(MP)->mnt_op->vfs_start)(MP, FLAGS, P)
#define VFS_UNMOUNT(MP, FORCE, P) (*(MP)->mnt_op->vfs_unmount)(MP, FORCE, P)
#define VFS_ROOT(MP, VPP)	  (*(MP)->mnt_op->vfs_root)(MP, VPP)
#define VFS_QUOTACTL(MP,C,U,A,P)  (*(MP)->mnt_op->vfs_quotactl)(MP, C, U, A, P)
#define VFS_STATVFS(MP, SBP, P)	  (*(MP)->mnt_op->vfs_statvfs)(MP, SBP, P)
#define VFS_SYNC(MP, WAIT, C, P)  (*(MP)->mnt_op->vfs_sync)(MP, WAIT, C, P)
#define VFS_VGET(MP, INO, VPP)	  (*(MP)->mnt_op->vfs_vget)(MP, INO, VPP)
#define VFS_FHTOVP(MP, FIDP, VPP) (*(MP)->mnt_op->vfs_fhtovp)(MP, FIDP, VPP)
#define VFS_CHECKEXP(MP, NAM, EXFLG, CRED) \
	(*(MP)->mnt_op->vfs_checkexp)(MP, NAM, EXFLG, CRED)
#define	VFS_VPTOFH(VP, FIDP)	  (*(VP)->v_mount->mnt_op->vfs_vptofh)(VP, FIDP)
#define VFS_SNAPSHOT(MP, VP, TS)  (*(MP)->mnt_op->vfs_snapshot)(MP, VP, TS)
#define	VFS_EXTATTRCTL(MP, C, VP, AS, AN, P) \
	(*(MP)->mnt_op->vfs_extattrctl)(MP, C, VP, AS, AN, P)
#endif /* _KERNEL */

#ifdef _KERNEL
#include <net/radix.h>
#include <sys/socket.h>		/* XXX for AF_MAX */

/*
 * Network address lookup element
 */
struct netcred {
	struct	radix_node netc_rnodes[2];
	int	netc_refcnt;
	int	netc_exflags;
	struct	ucred netc_anon;
};

/*
 * Network export information
 */
struct netexport {
	struct	netcred ne_defexported;		      /* Default export */
	struct	radix_node_head *ne_rtable[AF_MAX+1]; /* Individual exports */
};
#endif /* _KERNEL */

/*
 * Export arguments for local filesystem mount calls.
 */
struct export_args {
	int	ex_flags;		/* export related flags */
	uid_t	ex_root;		/* mapping for root uid */
	struct	uucred ex_anon;		/* mapping for anonymous user */
	struct	sockaddr *ex_addr;	/* net address to which exported */
	int	ex_addrlen;		/* and the net address length */
	struct	sockaddr *ex_mask;	/* mask of valid bits in saddr */
	int	ex_masklen;		/* and the smask length */
	char	*ex_indexfile;		/* index file for WebNFS URLs */
};

/*
 * Structure holding information for a publicly exported filesystem
 * (WebNFS). Currently the specs allow just for one such filesystem.
 */
struct nfs_public {
	int		np_valid;	/* Do we hold valid information */
	fhandle_t	np_handle;	/* Filehandle for pub fs (internal) */
	struct mount	*np_mount;	/* Mountpoint of exported fs */
	char		*np_index;	/* Index file */
};

#ifdef _KERNEL
#include <sys/mallocvar.h>
MALLOC_DECLARE(M_MOUNT);

/*
 * exported VFS interface (see vfssubr(9))
 */
struct	mount *vfs_getvfs(fsid_t *);    /* return vfs given fsid */
int	vfs_export			    /* process mount export info */
	 (struct mount *, struct netexport *, struct export_args *);
#define	vfs_showexport(a, b, c)	(void)memset((b), 0, sizeof(*(b)))
struct	netcred *vfs_export_lookup	    /* lookup host in fs export list */
	 (struct mount *, struct netexport *, struct mbuf *);
int	vfs_setpublicfs			    /* set publicly exported fs */
	 (struct mount *, struct netexport *, struct export_args *);
int	vfs_mountedon(struct vnode *);/* is a vfs mounted on vp */
int	vfs_mountroot(void);
void	vfs_shutdown(void);	    /* unmount and sync file systems */
void	vfs_unmountall(struct proc *);	    /* unmount file systems */
int 	vfs_busy(struct mount *, int, struct simplelock *);
int	vfs_rootmountalloc(char *, char *, struct mount **);
void	vfs_unbusy(struct mount *);
int	vfs_attach(struct vfsops *);
int	vfs_detach(struct vfsops *);
void	vfs_reinit(void);
struct vfsops *vfs_getopsbyname(const char *);

int	vfs_stdextattrctl(struct mount *, int, struct vnode *,
	    int, const char *, struct proc *);

extern	CIRCLEQ_HEAD(mntlist, mount) mountlist;	/* mounted filesystem list */
extern	struct vfsops *vfssw[];			/* filesystem type table */
extern	int nvfssw;
extern	struct nfs_public nfs_pub;
extern	struct simplelock mountlist_slock;
extern	struct simplelock spechash_slock;
long	makefstype(const char *);
int	dounmount(struct mount *, int, struct proc *);
void	vfsinit(void);
void	vfs_opv_init(const struct vnodeopv_desc * const *);
void	vfs_opv_free(const struct vnodeopv_desc * const *);
#ifdef DEBUG
void	vfs_bufstats(void);
#endif

LIST_HEAD(vfs_list_head, vfsops);
extern struct vfs_list_head vfs_list;

#else /* _KERNEL */

#include <sys/cdefs.h>

__BEGIN_DECLS
#ifdef __LIBC12_SOURCE__
int	fstatfs(int, struct statfs12 *);
int	getfsstat(struct statfs12 *, long, int);
int	statfs(const char *, struct statfs12 *);
int	getmntinfo(struct statfs12 **, int);
#endif
int	getfh(const char *, fhandle_t *);
int	mount(const char *, const char *, int, void *);
int	unmount(const char *, int);
#if defined(_NETBSD_SOURCE)
int	fhopen(const fhandle_t *, int);
int	fhstat(const fhandle_t *, struct stat *);
#ifdef __LIBC12_SOURCE__
int	fhstatfs(const fhandle_t *, struct statfs12 *);
#endif
#endif /* _NETBSD_SOURCE */
__END_DECLS

#endif /* _KERNEL */

#endif /* !_SYS_MOUNT_H_ */
