/*	$NetBSD: trap_stellix.h,v 1.1.1.4 2001/05/13 17:50:23 veego Exp $	*/

/* $srcdir/conf/trap/trap_stellix.h */
extern int mount_stellix(char *fsname, char *dir, int flags, int type, void *data);
#define	MOUNT_TRAP(type, mnt, flags, mnt_data) 	mount_stellix(mnt->mnt_fsname, mnt->mnt_dir, flags, type, mnt_data)
