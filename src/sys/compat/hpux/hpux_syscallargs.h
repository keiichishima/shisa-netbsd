/* $NetBSD: hpux_syscallargs.h,v 1.33 2005/02/26 23:58:19 perry Exp $ */

/*
 * System call argument lists.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * created from	NetBSD: syscalls.master,v 1.27 2003/01/18 07:36:58 thorpej Exp
 */

#ifndef _HPUX_SYS__SYSCALLARGS_H_
#define	_HPUX_SYS__SYSCALLARGS_H_

#ifdef	syscallarg
#undef	syscallarg
#endif

#define	syscallarg(x)							\
	union {								\
		register_t pad;						\
		struct { x datum; } le;					\
		struct { /* LINTED zero array dimension */		\
			int8_t pad[  /* CONSTCOND */			\
				(sizeof (register_t) < sizeof (x))	\
				? 0					\
				: sizeof (register_t) - sizeof (x)];	\
			x datum;					\
		} be;							\
	}

struct hpux_sys_read_args {
	syscallarg(int) fd;
	syscallarg(char *) buf;
	syscallarg(u_int) nbyte;
};

struct hpux_sys_write_args {
	syscallarg(int) fd;
	syscallarg(char *) buf;
	syscallarg(u_int) nbyte;
};

struct hpux_sys_open_args {
	syscallarg(const char *) path;
	syscallarg(int) flags;
	syscallarg(int) mode;
};

struct hpux_sys_wait_args {
	syscallarg(int *) status;
};

struct hpux_sys_creat_args {
	syscallarg(const char *) path;
	syscallarg(int) mode;
};

struct hpux_sys_unlink_args {
	syscallarg(const char *) path;
};

struct hpux_sys_execv_args {
	syscallarg(const char *) path;
	syscallarg(char **) argp;
};

struct hpux_sys_chdir_args {
	syscallarg(const char *) path;
};

struct hpux_sys_time_6x_args {
	syscallarg(time_t *) t;
};

struct hpux_sys_mknod_args {
	syscallarg(const char *) path;
	syscallarg(int) mode;
	syscallarg(int) dev;
};

struct hpux_sys_chmod_args {
	syscallarg(const char *) path;
	syscallarg(int) mode;
};

struct hpux_sys_chown_args {
	syscallarg(const char *) path;
	syscallarg(int) uid;
	syscallarg(int) gid;
};

struct hpux_sys_stat_6x_args {
	syscallarg(const char *) path;
	syscallarg(struct hpux_ostat *) sb;
};

struct hpux_sys_stime_6x_args {
	syscallarg(int) time;
};
#if 0

struct hpux_sys_ptrace_args {
	syscallarg(int) req;
	syscallarg(int) pid;
	syscallarg(int *) addr;
	syscallarg(int) data;
};
#else
#endif

struct hpux_sys_alarm_6x_args {
	syscallarg(int) deltat;
};

struct hpux_sys_fstat_6x_args {
	syscallarg(int) fd;
	syscallarg(struct hpux_ostat *) sb;
};

struct hpux_sys_utime_6x_args {
	syscallarg(char *) fname;
	syscallarg(time_t *) tptr;
};

struct hpux_sys_stty_6x_args {
	syscallarg(int) fd;
	syscallarg(caddr_t) arg;
};

struct hpux_sys_gtty_6x_args {
	syscallarg(int) fd;
	syscallarg(caddr_t) arg;
};

struct hpux_sys_access_args {
	syscallarg(const char *) path;
	syscallarg(int) flags;
};

struct hpux_sys_nice_6x_args {
	syscallarg(int) nval;
};

struct hpux_sys_ftime_6x_args {
	syscallarg(struct hpux_timeb *) tp;
};

struct hpux_sys_kill_args {
	syscallarg(pid_t) pid;
	syscallarg(int) signo;
};

struct hpux_sys_stat_args {
	syscallarg(const char *) path;
	syscallarg(struct hpux_stat *) sb;
};

struct hpux_sys_lstat_args {
	syscallarg(const char *) path;
	syscallarg(struct hpux_stat *) sb;
};

struct hpux_sys_times_6x_args {
	syscallarg(struct tms *) tms;
};

struct hpux_sys_ssig_6x_args {
	syscallarg(int) signo;
	syscallarg(sig_t) fun;
};

struct hpux_sys_ioctl_args {
	syscallarg(int) fd;
	syscallarg(int) com;
	syscallarg(caddr_t) data;
};

struct hpux_sys_symlink_args {
	syscallarg(const char *) path;
	syscallarg(const char *) link;
};

struct hpux_sys_utssys_args {
	syscallarg(struct hpux_utsname *) uts;
	syscallarg(int) dev;
	syscallarg(int) request;
};

struct hpux_sys_readlink_args {
	syscallarg(const char *) path;
	syscallarg(char *) buf;
	syscallarg(int) count;
};

struct hpux_sys_execve_args {
	syscallarg(const char *) path;
	syscallarg(char **) argp;
	syscallarg(char **) envp;
};

struct hpux_sys_fcntl_args {
	syscallarg(int) fd;
	syscallarg(int) cmd;
	syscallarg(int) arg;
};

struct hpux_sys_ulimit_args {
	syscallarg(int) cmd;
	syscallarg(int) newlimit;
};

struct hpux_sys_mmap_args {
	syscallarg(caddr_t) addr;
	syscallarg(size_t) len;
	syscallarg(int) prot;
	syscallarg(int) flags;
	syscallarg(int) fd;
	syscallarg(long) pos;
};

struct hpux_sys_getpgrp2_args {
	syscallarg(pid_t) pid;
};

struct hpux_sys_setpgrp2_args {
	syscallarg(pid_t) pid;
	syscallarg(pid_t) pgid;
};

struct hpux_sys_wait3_args {
	syscallarg(int *) status;
	syscallarg(int) options;
	syscallarg(int) rusage;
};

struct hpux_sys_fstat_args {
	syscallarg(int) fd;
	syscallarg(struct hpux_stat *) sb;
};

struct hpux_sys_sigreturn_args {
	syscallarg(struct hpuxsigcontext *) sigcntxp;
};

struct hpux_sys_sigvec_args {
	syscallarg(int) signo;
	syscallarg(struct sigvec *) nsv;
	syscallarg(struct sigvec *) osv;
};

struct hpux_sys_sigblock_args {
	syscallarg(int) mask;
};

struct hpux_sys_sigsetmask_args {
	syscallarg(int) mask;
};

struct hpux_sys_sigpause_args {
	syscallarg(int) mask;
};

struct hpux_sys_readv_args {
	syscallarg(int) fd;
	syscallarg(struct iovec *) iovp;
	syscallarg(u_int) iovcnt;
};

struct hpux_sys_writev_args {
	syscallarg(int) fd;
	syscallarg(struct iovec *) iovp;
	syscallarg(u_int) iovcnt;
};

struct hpux_sys_setresuid_args {
	syscallarg(uid_t) r;
	syscallarg(uid_t) e;
	syscallarg(uid_t) s;
};

struct hpux_sys_setresgid_args {
	syscallarg(gid_t) r;
	syscallarg(gid_t) e;
	syscallarg(gid_t) s;
};

struct hpux_sys_rename_args {
	syscallarg(const char *) from;
	syscallarg(const char *) to;
};

struct hpux_sys_truncate_args {
	syscallarg(const char *) path;
	syscallarg(long) length;
};

struct hpux_sys_sysconf_args {
	syscallarg(int) name;
};

struct hpux_sys_mkdir_args {
	syscallarg(const char *) path;
	syscallarg(int) mode;
};

struct hpux_sys_rmdir_args {
	syscallarg(const char *) path;
};

struct hpux_sys_getrlimit_args {
	syscallarg(u_int) which;
	syscallarg(struct orlimit *) rlp;
};

struct hpux_sys_setrlimit_args {
	syscallarg(u_int) which;
	syscallarg(struct orlimit *) rlp;
};

struct hpux_sys_rtprio_args {
	syscallarg(pid_t) pid;
	syscallarg(int) prio;
};

struct hpux_sys_netioctl_args {
	syscallarg(int) call;
	syscallarg(int *) args;
};

struct hpux_sys_lockf_args {
	syscallarg(int) fd;
	syscallarg(int) func;
	syscallarg(long) size;
};
#ifdef SYSVSEM

struct hpux_sys_osemctl_args {
	syscallarg(int) semid;
	syscallarg(int) semnum;
	syscallarg(int) cmd;
	syscallarg(union __semun) arg;
};
#else
#endif
#ifdef SYSVMSG

struct hpux_sys_omsgctl_args {
	syscallarg(int) msqid;
	syscallarg(int) cmd;
	syscallarg(struct hpux_omsqid_ds *) buf;
};
#else
#endif
#ifdef SYSVSHM

struct hpux_sys_oshmctl_args {
	syscallarg(int) shmid;
	syscallarg(int) cmd;
	syscallarg(struct hpux_oshmid_ds *) buf;
};
#else
#endif

struct hpux_sys_advise_args {
	syscallarg(int) arg;
};

struct hpux_sys_getcontext_args {
	syscallarg(char *) buf;
	syscallarg(int) len;
};

struct hpux_sys_getaccess_args {
	syscallarg(char *) path;
	syscallarg(uid_t) uid;
	syscallarg(int) ngroups;
	syscallarg(gid_t *) gidset;
	syscallarg(void *) label;
	syscallarg(void *) privs;
};

struct hpux_sys_waitpid_args {
	syscallarg(pid_t) pid;
	syscallarg(int *) status;
	syscallarg(int) options;
	syscallarg(struct rusage *) rusage;
};

struct hpux_sys_sigaction_args {
	syscallarg(int) signo;
	syscallarg(struct hpux_sigaction *) nsa;
	syscallarg(struct hpux_sigaction *) osa;
};

struct hpux_sys_sigprocmask_args {
	syscallarg(int) how;
	syscallarg(hpux_sigset_t *) set;
	syscallarg(hpux_sigset_t *) oset;
};

struct hpux_sys_sigpending_args {
	syscallarg(hpux_sigset_t *) set;
};

struct hpux_sys_sigsuspend_args {
	syscallarg(hpux_sigset_t *) set;
};

struct hpux_sys_setsockopt2_args {
	syscallarg(int) s;
	syscallarg(int) level;
	syscallarg(int) name;
	syscallarg(caddr_t) val;
	syscallarg(int) valsize;
};
#ifdef SYSVSEM

struct hpux_sys_semctl_args {
	syscallarg(int) semid;
	syscallarg(int) semnum;
	syscallarg(int) cmd;
	syscallarg(union __semun) arg;
};
#else
#endif
#ifdef SYSVMSG

struct hpux_sys_msgctl_args {
	syscallarg(int) msqid;
	syscallarg(int) cmd;
	syscallarg(struct hpux_msqid_ds *) buf;
};
#else
#endif
#ifdef SYSVSHM

struct hpux_sys_shmctl_args {
	syscallarg(int) shmid;
	syscallarg(int) cmd;
	syscallarg(struct hpux_shmid_ds *) buf;
};
#else
#endif

/*
 * System call prototypes.
 */

int	sys_nosys(struct lwp *, void *, register_t *);

int	sys_exit(struct lwp *, void *, register_t *);

int	hpux_sys_fork(struct lwp *, void *, register_t *);

int	hpux_sys_read(struct lwp *, void *, register_t *);

int	hpux_sys_write(struct lwp *, void *, register_t *);

int	hpux_sys_open(struct lwp *, void *, register_t *);

int	sys_close(struct lwp *, void *, register_t *);

int	hpux_sys_wait(struct lwp *, void *, register_t *);

int	hpux_sys_creat(struct lwp *, void *, register_t *);

int	sys_link(struct lwp *, void *, register_t *);

int	hpux_sys_unlink(struct lwp *, void *, register_t *);

int	hpux_sys_execv(struct lwp *, void *, register_t *);

int	hpux_sys_chdir(struct lwp *, void *, register_t *);

int	hpux_sys_time_6x(struct lwp *, void *, register_t *);

int	hpux_sys_mknod(struct lwp *, void *, register_t *);

int	hpux_sys_chmod(struct lwp *, void *, register_t *);

int	hpux_sys_chown(struct lwp *, void *, register_t *);

int	sys_obreak(struct lwp *, void *, register_t *);

int	hpux_sys_stat_6x(struct lwp *, void *, register_t *);

int	compat_43_sys_lseek(struct lwp *, void *, register_t *);

int	sys_getpid(struct lwp *, void *, register_t *);

int	sys_setuid(struct lwp *, void *, register_t *);

int	sys_getuid(struct lwp *, void *, register_t *);

int	hpux_sys_stime_6x(struct lwp *, void *, register_t *);

#if 0
int	hpux_sys_ptrace(struct lwp *, void *, register_t *);

#else
#endif
int	hpux_sys_alarm_6x(struct lwp *, void *, register_t *);

int	hpux_sys_fstat_6x(struct lwp *, void *, register_t *);

int	hpux_sys_pause_6x(struct lwp *, void *, register_t *);

int	hpux_sys_utime_6x(struct lwp *, void *, register_t *);

int	hpux_sys_stty_6x(struct lwp *, void *, register_t *);

int	hpux_sys_gtty_6x(struct lwp *, void *, register_t *);

int	hpux_sys_access(struct lwp *, void *, register_t *);

int	hpux_sys_nice_6x(struct lwp *, void *, register_t *);

int	hpux_sys_ftime_6x(struct lwp *, void *, register_t *);

int	sys_sync(struct lwp *, void *, register_t *);

int	hpux_sys_kill(struct lwp *, void *, register_t *);

int	hpux_sys_stat(struct lwp *, void *, register_t *);

int	hpux_sys_setpgrp_6x(struct lwp *, void *, register_t *);

int	hpux_sys_lstat(struct lwp *, void *, register_t *);

int	sys_dup(struct lwp *, void *, register_t *);

int	sys_pipe(struct lwp *, void *, register_t *);

int	hpux_sys_times_6x(struct lwp *, void *, register_t *);

int	sys_profil(struct lwp *, void *, register_t *);

int	sys_setgid(struct lwp *, void *, register_t *);

int	sys_getgid(struct lwp *, void *, register_t *);

int	hpux_sys_ssig_6x(struct lwp *, void *, register_t *);

int	hpux_sys_ioctl(struct lwp *, void *, register_t *);

int	hpux_sys_symlink(struct lwp *, void *, register_t *);

int	hpux_sys_utssys(struct lwp *, void *, register_t *);

int	hpux_sys_readlink(struct lwp *, void *, register_t *);

int	hpux_sys_execve(struct lwp *, void *, register_t *);

int	sys_umask(struct lwp *, void *, register_t *);

int	sys_chroot(struct lwp *, void *, register_t *);

int	hpux_sys_fcntl(struct lwp *, void *, register_t *);

int	hpux_sys_ulimit(struct lwp *, void *, register_t *);

int	hpux_sys_vfork(struct lwp *, void *, register_t *);

int	hpux_sys_mmap(struct lwp *, void *, register_t *);

int	sys_munmap(struct lwp *, void *, register_t *);

int	sys_mprotect(struct lwp *, void *, register_t *);

int	sys_getgroups(struct lwp *, void *, register_t *);

int	sys_setgroups(struct lwp *, void *, register_t *);

int	hpux_sys_getpgrp2(struct lwp *, void *, register_t *);

int	hpux_sys_setpgrp2(struct lwp *, void *, register_t *);

int	sys_setitimer(struct lwp *, void *, register_t *);

int	hpux_sys_wait3(struct lwp *, void *, register_t *);

int	sys_getitimer(struct lwp *, void *, register_t *);

int	sys_dup2(struct lwp *, void *, register_t *);

int	hpux_sys_fstat(struct lwp *, void *, register_t *);

int	sys_select(struct lwp *, void *, register_t *);

int	sys_fsync(struct lwp *, void *, register_t *);

int	hpux_sys_sigreturn(struct lwp *, void *, register_t *);

int	hpux_sys_sigvec(struct lwp *, void *, register_t *);

int	hpux_sys_sigblock(struct lwp *, void *, register_t *);

int	hpux_sys_sigsetmask(struct lwp *, void *, register_t *);

int	hpux_sys_sigpause(struct lwp *, void *, register_t *);

int	compat_43_sys_sigstack(struct lwp *, void *, register_t *);

int	sys_gettimeofday(struct lwp *, void *, register_t *);

int	hpux_sys_readv(struct lwp *, void *, register_t *);

int	hpux_sys_writev(struct lwp *, void *, register_t *);

int	sys_settimeofday(struct lwp *, void *, register_t *);

int	sys___posix_fchown(struct lwp *, void *, register_t *);

int	sys_fchmod(struct lwp *, void *, register_t *);

int	hpux_sys_setresuid(struct lwp *, void *, register_t *);

int	hpux_sys_setresgid(struct lwp *, void *, register_t *);

int	hpux_sys_rename(struct lwp *, void *, register_t *);

int	hpux_sys_truncate(struct lwp *, void *, register_t *);

int	compat_43_sys_ftruncate(struct lwp *, void *, register_t *);

int	hpux_sys_sysconf(struct lwp *, void *, register_t *);

int	hpux_sys_mkdir(struct lwp *, void *, register_t *);

int	hpux_sys_rmdir(struct lwp *, void *, register_t *);

int	hpux_sys_getrlimit(struct lwp *, void *, register_t *);

int	hpux_sys_setrlimit(struct lwp *, void *, register_t *);

int	hpux_sys_rtprio(struct lwp *, void *, register_t *);

int	hpux_sys_netioctl(struct lwp *, void *, register_t *);

int	hpux_sys_lockf(struct lwp *, void *, register_t *);

#ifdef SYSVSEM
int	sys_semget(struct lwp *, void *, register_t *);

int	hpux_sys_osemctl(struct lwp *, void *, register_t *);

int	sys_semop(struct lwp *, void *, register_t *);

#else
#endif
#ifdef SYSVMSG
int	sys_msgget(struct lwp *, void *, register_t *);

int	hpux_sys_omsgctl(struct lwp *, void *, register_t *);

int	sys_msgsnd(struct lwp *, void *, register_t *);

int	sys_msgrcv(struct lwp *, void *, register_t *);

#else
#endif
#ifdef SYSVSHM
int	sys_shmget(struct lwp *, void *, register_t *);

int	hpux_sys_oshmctl(struct lwp *, void *, register_t *);

int	sys_shmat(struct lwp *, void *, register_t *);

int	sys_shmdt(struct lwp *, void *, register_t *);

#else
#endif
int	hpux_sys_advise(struct lwp *, void *, register_t *);

int	hpux_sys_getcontext(struct lwp *, void *, register_t *);

int	hpux_sys_getaccess(struct lwp *, void *, register_t *);

int	hpux_sys_waitpid(struct lwp *, void *, register_t *);

int	sys_pathconf(struct lwp *, void *, register_t *);

int	sys_fpathconf(struct lwp *, void *, register_t *);

int	compat_43_sys_getdirentries(struct lwp *, void *, register_t *);

int	compat_09_sys_getdomainname(struct lwp *, void *, register_t *);

int	compat_09_sys_setdomainname(struct lwp *, void *, register_t *);

int	hpux_sys_sigaction(struct lwp *, void *, register_t *);

int	hpux_sys_sigprocmask(struct lwp *, void *, register_t *);

int	hpux_sys_sigpending(struct lwp *, void *, register_t *);

int	hpux_sys_sigsuspend(struct lwp *, void *, register_t *);

int	compat_43_sys_getdtablesize(struct lwp *, void *, register_t *);

int	sys_poll(struct lwp *, void *, register_t *);

int	sys_fchdir(struct lwp *, void *, register_t *);

int	compat_43_sys_accept(struct lwp *, void *, register_t *);

int	sys_bind(struct lwp *, void *, register_t *);

int	sys_connect(struct lwp *, void *, register_t *);

int	compat_43_sys_getpeername(struct lwp *, void *, register_t *);

int	compat_43_sys_getsockname(struct lwp *, void *, register_t *);

int	sys_getsockopt(struct lwp *, void *, register_t *);

int	sys_listen(struct lwp *, void *, register_t *);

int	compat_43_sys_recv(struct lwp *, void *, register_t *);

int	compat_43_sys_recvfrom(struct lwp *, void *, register_t *);

int	compat_43_sys_recvmsg(struct lwp *, void *, register_t *);

int	compat_43_sys_send(struct lwp *, void *, register_t *);

int	compat_43_sys_sendmsg(struct lwp *, void *, register_t *);

int	sys_sendto(struct lwp *, void *, register_t *);

int	hpux_sys_setsockopt2(struct lwp *, void *, register_t *);

int	sys_shutdown(struct lwp *, void *, register_t *);

int	sys_socket(struct lwp *, void *, register_t *);

int	sys_socketpair(struct lwp *, void *, register_t *);

#ifdef SYSVSEM
int	hpux_sys_semctl(struct lwp *, void *, register_t *);

#else
#endif
#ifdef SYSVMSG
int	hpux_sys_msgctl(struct lwp *, void *, register_t *);

#else
#endif
#ifdef SYSVSHM
int	hpux_sys_shmctl(struct lwp *, void *, register_t *);

#else
#endif
#endif /* _HPUX_SYS__SYSCALLARGS_H_ */
