/*	$NetBSD: lfs_syscalls.c,v 1.103.2.1 2005/05/07 11:21:30 tron Exp $	*/

/*-
 * Copyright (c) 1999, 2000, 2001, 2002, 2003 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Konrad E. Schroder <perseant@hhhh.org>.
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
 *	This product includes software developed by the NetBSD
 *	Foundation, Inc. and its contributors.
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
/*-
 * Copyright (c) 1991, 1993, 1994
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
 *	@(#)lfs_syscalls.c	8.10 (Berkeley) 5/14/95
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: lfs_syscalls.c,v 1.103.2.1 2005/05/07 11:21:30 tron Exp $");

#ifndef LFS
# define LFS		/* for prototypes in syscallargs.h */
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/kernel.h>

#include <sys/sa.h>
#include <sys/syscallargs.h>

#include <ufs/ufs/inode.h>
#include <ufs/ufs/ufsmount.h>
#include <ufs/ufs/ufs_extern.h>

#include <ufs/lfs/lfs.h>
#include <ufs/lfs/lfs_extern.h>

struct buf *lfs_fakebuf(struct lfs *, struct vnode *, int, size_t, caddr_t);
int lfs_fasthashget(dev_t, ino_t, struct vnode **);

pid_t lfs_cleaner_pid = 0;

#define LFS_FORCE_WRITE UNASSIGNED

/*
 * sys_lfs_markv:
 *
 * This will mark inodes and blocks dirty, so they are written into the log.
 * It will block until all the blocks have been written.  The segment create
 * time passed in the block_info and inode_info structures is used to decide
 * if the data is valid for each block (in case some process dirtied a block
 * or inode that is being cleaned between the determination that a block is
 * live and the lfs_markv call).
 *
 *  0 on success
 * -1/errno is return on error.
 */
#ifdef USE_64BIT_SYSCALLS
int
sys_lfs_markv(struct proc *p, void *v, register_t *retval)
{
	struct sys_lfs_markv_args /* {
		syscallarg(fsid_t *) fsidp;
		syscallarg(struct block_info *) blkiov;
		syscallarg(int) blkcnt;
	} */ *uap = v;
	BLOCK_INFO *blkiov;
	int blkcnt, error;
	fsid_t fsid;
	struct lfs *fs;
	struct mount *mntp;

	if ((error = suser(p->p_ucred, &p->p_acflag)) != 0)
		return (error);

	if ((error = copyin(SCARG(uap, fsidp), &fsid, sizeof(fsid_t))) != 0)
		return (error);

	if ((mntp = vfs_getvfs(fsidp)) == NULL) 
		return (ENOENT);
	fs = VFSTOUFS(mntp)->um_lfs;

	blkcnt = SCARG(uap, blkcnt);
	if ((u_int) blkcnt > LFS_MARKV_MAXBLKCNT)
		return (EINVAL);

	blkiov = lfs_malloc(fs, blkcnt * sizeof(BLOCK_INFO), LFS_NB_BLKIOV);
	if ((error = copyin(SCARG(uap, blkiov), blkiov,
			    blkcnt * sizeof(BLOCK_INFO))) != 0)
		goto out;

	if ((error = lfs_markv(p, &fsid, blkiov, blkcnt)) == 0)
		copyout(blkiov, SCARG(uap, blkiov),
			blkcnt * sizeof(BLOCK_INFO));
    out:
	lfs_free(fs, blkiov, LFS_NB_BLKIOV);
	return error;
}
#else
int
sys_lfs_markv(struct lwp *l, void *v, register_t *retval)
{
	struct sys_lfs_markv_args /* {
		syscallarg(fsid_t *) fsidp;
		syscallarg(struct block_info *) blkiov;
		syscallarg(int) blkcnt;
	} */ *uap = v;
	BLOCK_INFO *blkiov;
	BLOCK_INFO_15 *blkiov15;
	int i, blkcnt, error;
	fsid_t fsid;
	struct lfs *fs;
	struct mount *mntp;

	if ((error = suser(l->l_proc->p_ucred, &l->l_proc->p_acflag)) != 0)
		return (error);

	if ((error = copyin(SCARG(uap, fsidp), &fsid, sizeof(fsid_t))) != 0)
		return (error);

	if ((mntp = vfs_getvfs(&fsid)) == NULL) 
		return (ENOENT);
	fs = VFSTOUFS(mntp)->um_lfs;

	blkcnt = SCARG(uap, blkcnt);
	if ((u_int) blkcnt > LFS_MARKV_MAXBLKCNT)
		return (EINVAL);

	blkiov = lfs_malloc(fs, blkcnt * sizeof(BLOCK_INFO), LFS_NB_BLKIOV);
	blkiov15 = lfs_malloc(fs, blkcnt * sizeof(BLOCK_INFO_15), LFS_NB_BLKIOV);
	if ((error = copyin(SCARG(uap, blkiov), blkiov15,
			    blkcnt * sizeof(BLOCK_INFO_15))) != 0)
		goto out;

	for (i = 0; i < blkcnt; i++) {
		blkiov[i].bi_inode     = blkiov15[i].bi_inode;
		blkiov[i].bi_lbn       = blkiov15[i].bi_lbn;
		blkiov[i].bi_daddr     = blkiov15[i].bi_daddr;
		blkiov[i].bi_segcreate = blkiov15[i].bi_segcreate;
		blkiov[i].bi_version   = blkiov15[i].bi_version;
		blkiov[i].bi_bp	       = blkiov15[i].bi_bp;
		blkiov[i].bi_size      = blkiov15[i].bi_size;
	}

	if ((error = lfs_markv(l->l_proc, &fsid, blkiov, blkcnt)) == 0) {
		for (i = 0; i < blkcnt; i++) {
			blkiov15[i].bi_inode	 = blkiov[i].bi_inode;
			blkiov15[i].bi_lbn	 = blkiov[i].bi_lbn;
			blkiov15[i].bi_daddr	 = blkiov[i].bi_daddr;
			blkiov15[i].bi_segcreate = blkiov[i].bi_segcreate;
			blkiov15[i].bi_version	 = blkiov[i].bi_version;
			blkiov15[i].bi_bp	 = blkiov[i].bi_bp;
			blkiov15[i].bi_size	 = blkiov[i].bi_size;
		}
		copyout(blkiov15, SCARG(uap, blkiov),
			blkcnt * sizeof(BLOCK_INFO_15));
	}
    out:
	lfs_free(fs, blkiov, LFS_NB_BLKIOV);
	lfs_free(fs, blkiov15, LFS_NB_BLKIOV);
	return error;
}
#endif

#define	LFS_MARKV_MAX_BLOCKS	(LFS_MAX_BUFS)

int
lfs_markv(struct proc *p, fsid_t *fsidp, BLOCK_INFO *blkiov, int blkcnt)
{
	BLOCK_INFO *blkp;
	IFILE *ifp;
	struct buf *bp;
	struct inode *ip = NULL;
	struct lfs *fs;
	struct mount *mntp;
	struct vnode *vp;
	ino_t lastino;
	daddr_t b_daddr, v_daddr;
	int cnt, error;
	int do_again = 0;
	int numrefed = 0;
	ino_t maxino;
	size_t obsize;

	/* number of blocks/inodes that we have already bwrite'ed */
	int nblkwritten, ninowritten;

	if ((mntp = vfs_getvfs(fsidp)) == NULL)
		return (ENOENT);

	fs = VFSTOUFS(mntp)->um_lfs;

	if (fs->lfs_ronly)
		return EROFS;

	maxino = (fragstoblks(fs, fsbtofrags(fs, VTOI(fs->lfs_ivnode)->i_ffs1_blocks)) -
		      fs->lfs_cleansz - fs->lfs_segtabsz) * fs->lfs_ifpb;

	cnt = blkcnt;

	if ((error = vfs_busy(mntp, LK_NOWAIT, NULL)) != 0)
		return (error);

	/*
	 * This seglock is just to prevent the fact that we might have to sleep
	 * from allowing the possibility that our blocks might become
	 * invalid.
	 *
	 * It is also important to note here that unless we specify SEGM_CKP,
	 * any Ifile blocks that we might be asked to clean will never get
	 * to the disk.
	 */
	lfs_seglock(fs, SEGM_CLEAN | SEGM_CKP | SEGM_SYNC);

	/* Mark blocks/inodes dirty.  */
	error = 0;

	/* these were inside the initialization for the for loop */
	v_daddr = LFS_UNUSED_DADDR;
	lastino = LFS_UNUSED_INUM;
	nblkwritten = ninowritten = 0;
	for (blkp = blkiov; cnt--; ++blkp)
	{
		if (blkp->bi_daddr == LFS_FORCE_WRITE)
			DLOG((DLOG_CLEAN, "lfs_markv: warning: force-writing"
			      " ino %d lbn %lld\n", blkp->bi_inode,
			      (long long)blkp->bi_lbn));
		/* Bounds-check incoming data, avoid panic for failed VGET */
		if (blkp->bi_inode <= 0 || blkp->bi_inode >= maxino) {
			error = EINVAL;
			goto err3;
		}
		/*
		 * Get the IFILE entry (only once) and see if the file still
		 * exists.
		 */
		if (lastino != blkp->bi_inode) {
			/*
			 * Finish the old file, if there was one.  The presence
			 * of a usable vnode in vp is signaled by a valid v_daddr.
			 */
			if (v_daddr != LFS_UNUSED_DADDR) {
				lfs_vunref(vp);
				numrefed--;
			}

			/*
			 * Start a new file
			 */
			lastino = blkp->bi_inode;
			if (blkp->bi_inode == LFS_IFILE_INUM)
				v_daddr = fs->lfs_idaddr;
			else {
				LFS_IENTRY(ifp, fs, blkp->bi_inode, bp);
				/* XXX fix for force write */
				v_daddr = ifp->if_daddr;
				brelse(bp);
			}
			/* Don't force-write the ifile */
			if (blkp->bi_inode == LFS_IFILE_INUM
			    && blkp->bi_daddr == LFS_FORCE_WRITE)
			{
				continue;
			}
			if (v_daddr == LFS_UNUSED_DADDR
			    && blkp->bi_daddr != LFS_FORCE_WRITE)
			{
				continue;
			}

			/* Get the vnode/inode. */
			error = lfs_fastvget(mntp, blkp->bi_inode, v_daddr,
					   &vp,
					   (blkp->bi_lbn == LFS_UNUSED_LBN
					    ? blkp->bi_bp
					    : NULL));

			if (!error) {
				numrefed++;
			}
			if (error) {
				DLOG((DLOG_CLEAN, "lfs_markv: lfs_fastvget"
				      " failed with %d (ino %d, segment %d)\n",
				      error, blkp->bi_inode,
				      dtosn(fs, blkp->bi_daddr)));
				/*
				 * If we got EAGAIN, that means that the
				 * Inode was locked.  This is
				 * recoverable: just clean the rest of
				 * this segment, and let the cleaner try
				 * again with another.	(When the
				 * cleaner runs again, this segment will
				 * sort high on the list, since it is
				 * now almost entirely empty.) But, we
				 * still set v_daddr = LFS_UNUSED_ADDR
				 * so as not to test this over and over
				 * again.
				 */
				if (error == EAGAIN) {
					error = 0;
					do_again++;
				}
#ifdef DIAGNOSTIC
				else if (error != ENOENT)
					panic("lfs_markv VFS_VGET FAILED");
#endif
				/* lastino = LFS_UNUSED_INUM; */
				v_daddr = LFS_UNUSED_DADDR;
				vp = NULL;
				ip = NULL;
				continue;
			}
			ip = VTOI(vp);
			ninowritten++;
		} else if (v_daddr == LFS_UNUSED_DADDR) {
			/*
			 * This can only happen if the vnode is dead (or
			 * in any case we can't get it...e.g., it is
			 * inlocked).  Keep going.
			 */
			continue;
		}

		/* Past this point we are guaranteed that vp, ip are valid. */

		/* If this BLOCK_INFO didn't contain a block, keep going. */
		if (blkp->bi_lbn == LFS_UNUSED_LBN) {
			/* XXX need to make sure that the inode gets written in this case */
			/* XXX but only write the inode if it's the right one */
			if (blkp->bi_inode != LFS_IFILE_INUM) {
				LFS_IENTRY(ifp, fs, blkp->bi_inode, bp);
				if (ifp->if_daddr == blkp->bi_daddr
				   || blkp->bi_daddr == LFS_FORCE_WRITE)
				{
					LFS_SET_UINO(ip, IN_CLEANING);
				}
				brelse(bp);
			}
			continue;
		}

		b_daddr = 0;
		if (blkp->bi_daddr != LFS_FORCE_WRITE) {
			if (VOP_BMAP(vp, blkp->bi_lbn, NULL, &b_daddr, NULL) ||
			    dbtofsb(fs, b_daddr) != blkp->bi_daddr)
			{
				if (dtosn(fs,dbtofsb(fs, b_daddr))
				   == dtosn(fs,blkp->bi_daddr))
				{
					DLOG((DLOG_CLEAN, "lfs_markv: wrong da same seg: %llx vs %llx\n",
					      (long long)blkp->bi_daddr, (long long)dbtofsb(fs, b_daddr)));
				}
				do_again++;
				continue;
			}
		}

		/*
		 * Check block sizes.  The blocks being cleaned come from
		 * disk, so they should have the same size as their on-disk
		 * counterparts.
		 */
		if (blkp->bi_lbn >= 0)
			obsize = blksize(fs, ip, blkp->bi_lbn);
		else
			obsize = fs->lfs_bsize;
		/* Check for fragment size change */
		if (blkp->bi_lbn >= 0 && blkp->bi_lbn < NDADDR) {
			obsize = ip->i_lfs_fragsize[blkp->bi_lbn];
		}
		if (obsize != blkp->bi_size) {
			DLOG((DLOG_CLEAN, "lfs_markv: ino %d lbn %lld wrong"
			      " size (%ld != %d), try again\n",
			      blkp->bi_inode, (long long)blkp->bi_lbn,
			      (long) obsize, blkp->bi_size));
			do_again++;
			continue;
		}

		/*
		 * If we get to here, then we are keeping the block.  If
		 * it is an indirect block, we want to actually put it
		 * in the buffer cache so that it can be updated in the
		 * finish_meta section.	 If it's not, we need to
		 * allocate a fake buffer so that writeseg can perform
		 * the copyin and write the buffer.
		 */
		if (ip->i_number != LFS_IFILE_INUM && blkp->bi_lbn >= 0) {
			/* Data Block */
			bp = lfs_fakebuf(fs, vp, blkp->bi_lbn,
					 blkp->bi_size, blkp->bi_bp);
			/* Pretend we used bread() to get it */
			bp->b_blkno = fsbtodb(fs, blkp->bi_daddr);
		} else {
			/* Indirect block or ifile */
			if (blkp->bi_size != fs->lfs_bsize &&
			    ip->i_number != LFS_IFILE_INUM)
				panic("lfs_markv: partial indirect block?"
				    " size=%d\n", blkp->bi_size);
			bp = getblk(vp, blkp->bi_lbn, blkp->bi_size, 0, 0);
			if (!(bp->b_flags & (B_DONE|B_DELWRI))) { /* B_CACHE */
				/*
				 * The block in question was not found
				 * in the cache; i.e., the block that
				 * getblk() returned is empty.	So, we
				 * can (and should) copy in the
				 * contents, because we've already
				 * determined that this was the right
				 * version of this block on disk.
				 *
				 * And, it can't have changed underneath
				 * us, because we have the segment lock.
				 */
				error = copyin(blkp->bi_bp, bp->b_data, blkp->bi_size);
				if (error)
					goto err2;
			}
		}
		if ((error = lfs_bwrite_ext(bp, BW_CLEAN)) != 0)
			goto err2;

		nblkwritten++;
		/*
		 * XXX should account indirect blocks and ifile pages as well
		 */
		if (nblkwritten + lblkno(fs, ninowritten * sizeof (struct ufs1_dinode))
		    > LFS_MARKV_MAX_BLOCKS) {
			DLOG((DLOG_CLEAN, "lfs_markv: writing %d blks %d inos\n",
			      nblkwritten, ninowritten));
			lfs_segwrite(mntp, SEGM_CLEAN);
			nblkwritten = ninowritten = 0;
		}
	}

	/*
	 * Finish the old file, if there was one
	 */
	if (v_daddr != LFS_UNUSED_DADDR) {
		lfs_vunref(vp);
		numrefed--;
	}

#ifdef DIAGNOSTIC
	if (numrefed != 0)
		panic("lfs_markv: numrefed=%d", numrefed);
#endif
	DLOG((DLOG_CLEAN, "lfs_markv: writing %d blks %d inos (check point)\n",
	      nblkwritten, ninowritten));

	/*
	 * The last write has to be SEGM_SYNC, because of calling semantics.
	 * It also has to be SEGM_CKP, because otherwise we could write
	 * over the newly cleaned data contained in a checkpoint, and then
	 * we'd be unhappy at recovery time.
	 */
	lfs_segwrite(mntp, SEGM_CLEAN | SEGM_CKP | SEGM_SYNC);

	lfs_segunlock(fs);

	vfs_unbusy(mntp);
	if (error)
		return (error);
	else if (do_again)
		return EAGAIN;

	return 0;

err2:
	DLOG((DLOG_CLEAN, "lfs_markv err2\n"));

	/*
	 * XXX we're here because copyin() failed.
	 * XXX it means that we can't trust the cleanerd.  too bad.
	 * XXX how can we recover from this?
	 */

err3:
	/*
	 * XXX should do segwrite here anyway?
	 */

	if (v_daddr != LFS_UNUSED_DADDR) {
		lfs_vunref(vp);
		--numrefed;
	}

	lfs_segunlock(fs);
	vfs_unbusy(mntp);
#ifdef DIAGNOSTIC
	if (numrefed != 0)
		panic("lfs_markv: numrefed=%d", numrefed);
#endif

	return (error);
}

/*
 * sys_lfs_bmapv:
 *
 * This will fill in the current disk address for arrays of blocks.
 *
 *  0 on success
 * -1/errno is return on error.
 */
#ifdef USE_64BIT_SYSCALLS
int
sys_lfs_bmapv(struct proc *p, void *v, register_t *retval)
{
	struct sys_lfs_bmapv_args /* {
		syscallarg(fsid_t *) fsidp;
		syscallarg(struct block_info *) blkiov;
		syscallarg(int) blkcnt;
	} */ *uap = v;
	BLOCK_INFO *blkiov;
	int blkcnt, error;
	fsid_t fsid;
	struct lfs *fs;
	struct mount *mntp;

	if ((error = suser(p->p_ucred, &p->p_acflag)) != 0)
		return (error);

	if ((error = copyin(SCARG(uap, fsidp), &fsid, sizeof(fsid_t))) != 0)
		return (error);

	if ((mntp = vfs_getvfs(&fsid)) == NULL) 
		return (ENOENT);
	fs = VFSTOUFS(mntp)->um_lfs;

	blkcnt = SCARG(uap, blkcnt);
	if ((u_int) blkcnt > SIZE_T_MAX / sizeof(BLOCK_INFO))
		return (EINVAL);
	blkiov = lfs_malloc(fs, blkcnt * sizeof(BLOCK_INFO), LFS_NB_BLKIOV);
	if ((error = copyin(SCARG(uap, blkiov), blkiov,
			    blkcnt * sizeof(BLOCK_INFO))) != 0)
		goto out;

	if ((error = lfs_bmapv(p, &fsid, blkiov, blkcnt)) == 0)
		copyout(blkiov, SCARG(uap, blkiov),
			blkcnt * sizeof(BLOCK_INFO));
    out:
	lfs_free(fs, blkiov, LFS_NB_BLKIOV);
	return error;
}
#else
int
sys_lfs_bmapv(struct lwp *l, void *v, register_t *retval)
{
	struct sys_lfs_bmapv_args /* {
		syscallarg(fsid_t *) fsidp;
		syscallarg(struct block_info *) blkiov;
		syscallarg(int) blkcnt;
	} */ *uap = v;
	struct proc *p = l->l_proc;
	BLOCK_INFO *blkiov;
	BLOCK_INFO_15 *blkiov15;
	int i, blkcnt, error;
	fsid_t fsid;
	struct lfs *fs;
	struct mount *mntp;

	if ((error = suser(p->p_ucred, &p->p_acflag)) != 0)
		return (error);

	if ((error = copyin(SCARG(uap, fsidp), &fsid, sizeof(fsid_t))) != 0)
		return (error);

	if ((mntp = vfs_getvfs(&fsid)) == NULL) 
		return (ENOENT);
	fs = VFSTOUFS(mntp)->um_lfs;

	blkcnt = SCARG(uap, blkcnt);
	if ((size_t) blkcnt > SIZE_T_MAX / sizeof(BLOCK_INFO))
		return (EINVAL);
	blkiov = lfs_malloc(fs, blkcnt * sizeof(BLOCK_INFO), LFS_NB_BLKIOV);
	blkiov15 = lfs_malloc(fs, blkcnt * sizeof(BLOCK_INFO_15), LFS_NB_BLKIOV);
	if ((error = copyin(SCARG(uap, blkiov), blkiov15,
			    blkcnt * sizeof(BLOCK_INFO_15))) != 0)
		goto out;

	for (i = 0; i < blkcnt; i++) {
		blkiov[i].bi_inode     = blkiov15[i].bi_inode;
		blkiov[i].bi_lbn       = blkiov15[i].bi_lbn;
		blkiov[i].bi_daddr     = blkiov15[i].bi_daddr;
		blkiov[i].bi_segcreate = blkiov15[i].bi_segcreate;
		blkiov[i].bi_version   = blkiov15[i].bi_version;
		blkiov[i].bi_bp	       = blkiov15[i].bi_bp;
		blkiov[i].bi_size      = blkiov15[i].bi_size;
	}

	if ((error = lfs_bmapv(p, &fsid, blkiov, blkcnt)) == 0) {
		for (i = 0; i < blkcnt; i++) {
			blkiov15[i].bi_inode	 = blkiov[i].bi_inode;
			blkiov15[i].bi_lbn	 = blkiov[i].bi_lbn;
			blkiov15[i].bi_daddr	 = blkiov[i].bi_daddr;
			blkiov15[i].bi_segcreate = blkiov[i].bi_segcreate;
			blkiov15[i].bi_version	 = blkiov[i].bi_version;
			blkiov15[i].bi_bp	 = blkiov[i].bi_bp;
			blkiov15[i].bi_size	 = blkiov[i].bi_size;
		}
		copyout(blkiov15, SCARG(uap, blkiov),
			blkcnt * sizeof(BLOCK_INFO_15));
	}
    out:
	lfs_free(fs, blkiov, LFS_NB_BLKIOV);
	lfs_free(fs, blkiov15, LFS_NB_BLKIOV);
	return error;
}
#endif

int
lfs_bmapv(struct proc *p, fsid_t *fsidp, BLOCK_INFO *blkiov, int blkcnt)
{
	BLOCK_INFO *blkp;
	IFILE *ifp;
	struct buf *bp;
	struct inode *ip = NULL;
	struct lfs *fs;
	struct mount *mntp;
	struct ufsmount *ump;
	struct vnode *vp;
	ino_t lastino;
	daddr_t v_daddr;
	int cnt, error;
	int numrefed = 0;

	lfs_cleaner_pid = p->p_pid;

	if ((mntp = vfs_getvfs(fsidp)) == NULL)
		return (ENOENT);

	ump = VFSTOUFS(mntp);
	if ((error = vfs_busy(mntp, LK_NOWAIT, NULL)) != 0)
		return (error);

	cnt = blkcnt;

	fs = VFSTOUFS(mntp)->um_lfs;

	error = 0;

	/* these were inside the initialization for the for loop */
	v_daddr = LFS_UNUSED_DADDR;
	lastino = LFS_UNUSED_INUM;
	for (blkp = blkiov; cnt--; ++blkp)
	{
		/*
		 * Get the IFILE entry (only once) and see if the file still
		 * exists.
		 */
		if (lastino != blkp->bi_inode) {
			/*
			 * Finish the old file, if there was one.  The presence
			 * of a usable vnode in vp is signaled by a valid
			 * v_daddr.
			 */
			if (v_daddr != LFS_UNUSED_DADDR) {
				lfs_vunref(vp);
				numrefed--;
			}

			/*
			 * Start a new file
			 */
			lastino = blkp->bi_inode;
			if (blkp->bi_inode == LFS_IFILE_INUM)
				v_daddr = fs->lfs_idaddr;
			else {
				LFS_IENTRY(ifp, fs, blkp->bi_inode, bp);
				v_daddr = ifp->if_daddr;
				brelse(bp);
			}
			if (v_daddr == LFS_UNUSED_DADDR) {
				blkp->bi_daddr = LFS_UNUSED_DADDR;
				continue;
			}
			/*
			 * A regular call to VFS_VGET could deadlock
			 * here.  Instead, we try an unlocked access.
			 */
			vp = ufs_ihashlookup(ump->um_dev, blkp->bi_inode);
			if (vp != NULL && !(vp->v_flag & VXLOCK)) {
				ip = VTOI(vp);
				if (lfs_vref(vp)) {
					v_daddr = LFS_UNUSED_DADDR;
					continue;
				}
				numrefed++;
			} else {
				/*
				 * Don't VFS_VGET if we're being unmounted,
				 * since we hold vfs_busy().
				 */
				if (mntp->mnt_iflag & IMNT_UNMOUNT) {
					v_daddr = LFS_UNUSED_DADDR;
					continue;
				}
				error = VFS_VGET(mntp, blkp->bi_inode, &vp);
				if (error) {
					DLOG((DLOG_CLEAN, "lfs_bmapv: vget ino"
					      "%d failed with %d",
					      blkp->bi_inode,error));
					v_daddr = LFS_UNUSED_DADDR;
					continue;
				} else {
					KASSERT(VOP_ISLOCKED(vp));
					VOP_UNLOCK(vp, 0);
					numrefed++;
				}
			}
			ip = VTOI(vp);
		} else if (v_daddr == LFS_UNUSED_DADDR) {
			/*
			 * This can only happen if the vnode is dead.
			 * Keep going.	Note that we DO NOT set the
			 * bi_addr to anything -- if we failed to get
			 * the vnode, for example, we want to assume
			 * conservatively that all of its blocks *are*
			 * located in the segment in question.
			 * lfs_markv will throw them out if we are
			 * wrong.
			 */
			/* blkp->bi_daddr = LFS_UNUSED_DADDR; */
			continue;
		}

		/* Past this point we are guaranteed that vp, ip are valid. */

		if (blkp->bi_lbn == LFS_UNUSED_LBN) {
			/*
			 * We just want the inode address, which is
			 * conveniently in v_daddr.
			 */
			blkp->bi_daddr = v_daddr;
		} else {
			daddr_t bi_daddr;

			/* XXX ondisk32 */
			error = VOP_BMAP(vp, blkp->bi_lbn, NULL,
					 &bi_daddr, NULL);
			if (error)
			{
				blkp->bi_daddr = LFS_UNUSED_DADDR;
				continue;
			}
			blkp->bi_daddr = dbtofsb(fs, bi_daddr);
			/* Fill in the block size, too */
			if (blkp->bi_lbn >= 0)
				blkp->bi_size = blksize(fs, ip, blkp->bi_lbn);
			else
				blkp->bi_size = fs->lfs_bsize;
		}
	}

	/*
	 * Finish the old file, if there was one.  The presence
	 * of a usable vnode in vp is signaled by a valid v_daddr.
	 */
	if (v_daddr != LFS_UNUSED_DADDR) {
		lfs_vunref(vp);
		numrefed--;
	}

#ifdef DIAGNOSTIC
	if (numrefed != 0)
		panic("lfs_bmapv: numrefed=%d", numrefed);
#endif

	vfs_unbusy(mntp);

	return 0;
}

/*
 * sys_lfs_segclean:
 *
 * Mark the segment clean.
 *
 *  0 on success
 * -1/errno is return on error.
 */
int
sys_lfs_segclean(struct lwp *l, void *v, register_t *retval)
{
	struct sys_lfs_segclean_args /* {
		syscallarg(fsid_t *) fsidp;
		syscallarg(u_long) segment;
	} */ *uap = v;
	struct lfs *fs;
	struct mount *mntp;
	fsid_t fsid;
	int error;
	unsigned long segnum;
	struct proc *p = l->l_proc;

	if ((error = suser(p->p_ucred, &p->p_acflag)) != 0)
		return (error);

	if ((error = copyin(SCARG(uap, fsidp), &fsid, sizeof(fsid_t))) != 0)
		return (error);
	if ((mntp = vfs_getvfs(&fsid)) == NULL)
		return (ENOENT);

	fs = VFSTOUFS(mntp)->um_lfs;
	segnum = SCARG(uap, segment);

	if ((error = vfs_busy(mntp, LK_NOWAIT, NULL)) != 0)
		return (error);

	lfs_seglock(fs, SEGM_PROT);
	error = lfs_do_segclean(fs, segnum);
	lfs_segunlock(fs);
	vfs_unbusy(mntp);
	return error;
}

/*
 * Actually mark the segment clean.
 * Must be called with the segment lock held.
 */
int
lfs_do_segclean(struct lfs *fs, unsigned long segnum)
{
	struct buf *bp;
	CLEANERINFO *cip;
	SEGUSE *sup;

	if (dtosn(fs, fs->lfs_curseg) == segnum) {
		return (EBUSY);
	}

	LFS_SEGENTRY(sup, fs, segnum, bp);
	if (sup->su_nbytes) {
		DLOG((DLOG_CLEAN, "lfs_segclean: not cleaning segment %lu:"
		      " %d live bytes\n", segnum, sup->su_nbytes));
		brelse(bp);
		return (EBUSY);
	}
	if (sup->su_flags & SEGUSE_ACTIVE) {
		brelse(bp);
		return (EBUSY);
	}
	if (!(sup->su_flags & SEGUSE_DIRTY)) {
		brelse(bp);
		return (EALREADY);
	}

	fs->lfs_avail += segtod(fs, 1);
	if (sup->su_flags & SEGUSE_SUPERBLOCK)
		fs->lfs_avail -= btofsb(fs, LFS_SBPAD);
	if (fs->lfs_version > 1 && segnum == 0 &&
	    fs->lfs_start < btofsb(fs, LFS_LABELPAD))
		fs->lfs_avail -= btofsb(fs, LFS_LABELPAD) - fs->lfs_start;
	simple_lock(&fs->lfs_interlock);
	fs->lfs_bfree += sup->su_nsums * btofsb(fs, fs->lfs_sumsize) +
		btofsb(fs, sup->su_ninos * fs->lfs_ibsize);
	simple_unlock(&fs->lfs_interlock);
	fs->lfs_dmeta -= sup->su_nsums * btofsb(fs, fs->lfs_sumsize) +
		btofsb(fs, sup->su_ninos * fs->lfs_ibsize);
	if (fs->lfs_dmeta < 0)
		fs->lfs_dmeta = 0;
	sup->su_flags &= ~SEGUSE_DIRTY;
	LFS_WRITESEGENTRY(sup, fs, segnum, bp);

	LFS_CLEANERINFO(cip, fs, bp);
	++cip->clean;
	--cip->dirty;
	fs->lfs_nclean = cip->clean;
	cip->bfree = fs->lfs_bfree;
	simple_lock(&fs->lfs_interlock);
	cip->avail = fs->lfs_avail - fs->lfs_ravail - fs->lfs_favail;
	simple_unlock(&fs->lfs_interlock);
	(void) LFS_BWRITE_LOG(bp);
	wakeup(&fs->lfs_avail);

	return (0);
}

/*
 * This will block until a segment in file system fsid is written.  A timeout
 * in milliseconds may be specified which will awake the cleaner automatically.
 * An fsid of -1 means any file system, and a timeout of 0 means forever.
 */
int
lfs_segwait(fsid_t *fsidp, struct timeval *tv)
{
	struct mount *mntp;
	void *addr;
	u_long timeout;
	int error, s;

	if ((mntp = vfs_getvfs(fsidp)) == NULL)
		addr = &lfs_allclean_wakeup;
	else
		addr = &VFSTOUFS(mntp)->um_lfs->lfs_nextseg;
	/*
	 * XXX THIS COULD SLEEP FOREVER IF TIMEOUT IS {0,0}!
	 * XXX IS THAT WHAT IS INTENDED?
	 */
	s = splclock();
	timeradd(tv, &time, tv);
	timeout = hzto(tv);
	splx(s);
	error = tsleep(addr, PCATCH | PUSER, "segment", timeout);
	return (error == ERESTART ? EINTR : 0);
}

/*
 * sys_lfs_segwait:
 *
 * System call wrapper around lfs_segwait().
 *
 *  0 on success
 *  1 on timeout
 * -1/errno is return on error.
 */
int
sys_lfs_segwait(struct lwp *l, void *v, register_t *retval)
{
	struct sys_lfs_segwait_args /* {
		syscallarg(fsid_t *) fsidp;
		syscallarg(struct timeval *) tv;
	} */ *uap = v;
	struct proc *p = l->l_proc;
	struct timeval atv;
	fsid_t fsid;
	int error;

	/* XXX need we be su to segwait? */
	if ((error = suser(p->p_ucred, &p->p_acflag)) != 0) {
		return (error);
	}
	if ((error = copyin(SCARG(uap, fsidp), &fsid, sizeof(fsid_t))) != 0)
		return (error);

	if (SCARG(uap, tv)) {
		error = copyin(SCARG(uap, tv), &atv, sizeof(struct timeval));
		if (error)
			return (error);
		if (itimerfix(&atv))
			return (EINVAL);
	} else /* NULL or invalid */
		atv.tv_sec = atv.tv_usec = 0;
	return lfs_segwait(&fsid, &atv);
}

/*
 * VFS_VGET call specialized for the cleaner.  The cleaner already knows the
 * daddr from the ifile, so don't look it up again.  If the cleaner is
 * processing IINFO structures, it may have the ondisk inode already, so
 * don't go retrieving it again.
 *
 * we lfs_vref, and it is the caller's responsibility to lfs_vunref
 * when finished.
 */
extern struct lock ufs_hashlock;

int
lfs_fasthashget(dev_t dev, ino_t ino, struct vnode **vpp)
{
	if ((*vpp = ufs_ihashlookup(dev, ino)) != NULL) {
		if ((*vpp)->v_flag & VXLOCK) {
			DLOG((DLOG_CLEAN, "lfs_fastvget: ino %d VXLOCK\n",
			      ino));
			lfs_stats.clean_vnlocked++;
			return EAGAIN;
		}
		if (lfs_vref(*vpp)) {
			DLOG((DLOG_CLEAN, "lfs_fastvget: lfs_vref failed"
			      " for ino %d\n", ino));
			lfs_stats.clean_inlocked++;
			return EAGAIN;
		}
	} else
		*vpp = NULL;

	return (0);
}

int
lfs_fastvget(struct mount *mp, ino_t ino, daddr_t daddr, struct vnode **vpp, struct ufs1_dinode *dinp)
{
	struct inode *ip;
	struct ufs1_dinode *dip;
	struct vnode *vp;
	struct ufsmount *ump;
	dev_t dev;
	int error, retries;
	struct buf *bp;
	struct lfs *fs;

	ump = VFSTOUFS(mp);
	dev = ump->um_dev;
	fs = ump->um_lfs;

	/*
	 * Wait until the filesystem is fully mounted before allowing vget
	 * to complete.	 This prevents possible problems with roll-forward.
	 */
	simple_lock(&fs->lfs_interlock);
	while (fs->lfs_flags & LFS_NOTYET) {
		ltsleep(&fs->lfs_flags, PRIBIO+1, "lfs_fnotyet", 0,
			&fs->lfs_interlock);
	}
	simple_unlock(&fs->lfs_interlock);

	/*
	 * This is playing fast and loose.  Someone may have the inode
	 * locked, in which case they are going to be distinctly unhappy
	 * if we trash something.
	 */

	error = lfs_fasthashget(dev, ino, vpp);
	if (error != 0 || *vpp != NULL)
		return (error);

	/*
	 * getnewvnode(9) will call vfs_busy, which will block if the
	 * filesystem is being unmounted; but umount(9) is waiting for
	 * us because we're already holding the fs busy.
	 * XXXMP
	 */
	if (mp->mnt_iflag & IMNT_UNMOUNT) {
		*vpp = NULL;
		return EDEADLK;
	}
	if ((error = getnewvnode(VT_LFS, mp, lfs_vnodeop_p, &vp)) != 0) {
		*vpp = NULL;
		return (error);
	}

	do {
		error = lfs_fasthashget(dev, ino, vpp);
		if (error != 0 || *vpp != NULL) {
			ungetnewvnode(vp);
			return (error);
		}
	} while (lockmgr(&ufs_hashlock, LK_EXCLUSIVE|LK_SLEEPFAIL, 0));

	/* Allocate new vnode/inode. */
	lfs_vcreate(mp, ino, vp);

	/*
	 * Put it onto its hash chain and lock it so that other requests for
	 * this inode will block if they arrive while we are sleeping waiting
	 * for old data structures to be purged or for the contents of the
	 * disk portion of this inode to be read.
	 */
	ip = VTOI(vp);
	ufs_ihashins(ip);
	lockmgr(&ufs_hashlock, LK_RELEASE, 0);

	/*
	 * XXX
	 * This may not need to be here, logically it should go down with
	 * the i_devvp initialization.
	 * Ask Kirk.
	 */
	ip->i_lfs = fs;

	/* Read in the disk contents for the inode, copy into the inode. */
	if (dinp) {
		error = copyin(dinp, ip->i_din.ffs1_din, sizeof (struct ufs1_dinode));
		if (error) {
			DLOG((DLOG_CLEAN, "lfs_fastvget: dinode copyin failed"
			      " for ino %d\n", ino));
			ufs_ihashrem(ip);

			/* Unlock and discard unneeded inode. */
			lockmgr(&vp->v_lock, LK_RELEASE, &vp->v_interlock);
			lfs_vunref(vp);
			*vpp = NULL;
			return (error);
		}
		if (ip->i_number != ino)
			panic("lfs_fastvget: I was fed the wrong inode!");
	} else {
		retries = 0;
	    again:
		error = bread(ump->um_devvp, fsbtodb(fs, daddr), fs->lfs_ibsize,
			      NOCRED, &bp);
		if (error) {
			DLOG((DLOG_CLEAN, "lfs_fastvget: bread failed (%d)\n",
			      error));
			/*
			 * The inode does not contain anything useful, so it
			 * would be misleading to leave it on its hash chain.
			 * Iput() will return it to the free list.
			 */
			ufs_ihashrem(ip);

			/* Unlock and discard unneeded inode. */
			lockmgr(&vp->v_lock, LK_RELEASE, &vp->v_interlock);
			lfs_vunref(vp);
			brelse(bp);
			*vpp = NULL;
			return (error);
		}
		dip = lfs_ifind(ump->um_lfs, ino, bp);
		if (dip == NULL) {
			/* Assume write has not completed yet; try again */
			bp->b_flags |= B_INVAL;
			brelse(bp);
			++retries;
			if (retries > LFS_IFIND_RETRIES)
				panic("lfs_fastvget: dinode not found");
			DLOG((DLOG_CLEAN, "lfs_fastvget: dinode not found,"
			      " retrying...\n"));
			goto again;
		}
		*ip->i_din.ffs1_din = *dip;
		brelse(bp);
	}
	lfs_vinit(mp, &vp);

	*vpp = vp;

	KASSERT(VOP_ISLOCKED(vp));
	VOP_UNLOCK(vp, 0);

	return (0);
}

/*
 * Make up a "fake" cleaner buffer, copy the data from userland into it.
 */
struct buf *
lfs_fakebuf(struct lfs *fs, struct vnode *vp, int lbn, size_t size, caddr_t uaddr)
{
	struct buf *bp;
	int error;

	KASSERT(VTOI(vp)->i_number != LFS_IFILE_INUM);

	bp = lfs_newbuf(VTOI(vp)->i_lfs, vp, lbn, size, LFS_NB_CLEAN);
	error = copyin(uaddr, bp->b_data, size);
	if (error) {
		lfs_freebuf(fs, bp);
		return NULL;
	}
	KDASSERT(bp->b_iodone == lfs_callback);

#if 0
	simple_lock(&fs->lfs_interlock);
	++fs->lfs_iocount;
	simple_unlock(&fs->lfs_interlock);
#endif
	bp->b_bufsize = size;
	bp->b_bcount = size;
	return (bp);
}
