/* $NetBSD: svr4_32_syscall.h,v 1.11 2005/02/26 23:58:20 perry Exp $ */

/*
 * System call numbers.
 *
 * DO NOT EDIT-- this file is automatically generated.
 * created from	NetBSD: syscalls.master,v 1.7 2003/12/07 01:36:58 dmcmahill Exp
 */

/* syscall: "syscall" ret: "int" args: */
#define	SVR4_32_SYS_syscall	0

/* syscall: "netbsd32_exit" ret: "int" args: "int" */
#define	SVR4_32_SYS_netbsd32_exit	1

/* syscall: "fork" ret: "int" args: */
#define	SVR4_32_SYS_fork	2

/* syscall: "netbsd32_read" ret: "int" args: "int" "netbsd32_charp" "u_int" */
#define	SVR4_32_SYS_netbsd32_read	3

/* syscall: "netbsd32_write" ret: "int" args: "int" "netbsd32_charp" "u_int" */
#define	SVR4_32_SYS_netbsd32_write	4

/* syscall: "open" ret: "int" args: "const netbsd32_charp" "int" "int" */
#define	SVR4_32_SYS_open	5

/* syscall: "netbsd32_close" ret: "int" args: "int" */
#define	SVR4_32_SYS_netbsd32_close	6

/* syscall: "wait" ret: "int" args: "netbsd32_intp" */
#define	SVR4_32_SYS_wait	7

/* syscall: "creat" ret: "int" args: "const netbsd32_charp" "int" */
#define	SVR4_32_SYS_creat	8

/* syscall: "netbsd32_link" ret: "int" args: "const netbsd32_charp" "const netbsd32_charp" */
#define	SVR4_32_SYS_netbsd32_link	9

/* syscall: "netbsd32_unlink" ret: "int" args: "const netbsd32_charp" */
#define	SVR4_32_SYS_netbsd32_unlink	10

/* syscall: "execv" ret: "int" args: "const netbsd32_charp" "netbsd32_charpp" */
#define	SVR4_32_SYS_execv	11

/* syscall: "netbsd32_chdir" ret: "int" args: "const netbsd32_charp" */
#define	SVR4_32_SYS_netbsd32_chdir	12

/* syscall: "time" ret: "int" args: "svr4_32_time_tp" */
#define	SVR4_32_SYS_time	13

/* syscall: "mknod" ret: "int" args: "const netbsd32_charp" "int" "int" */
#define	SVR4_32_SYS_mknod	14

/* syscall: "netbsd32_chmod" ret: "int" args: "const netbsd32_charp" "mode_t" */
#define	SVR4_32_SYS_netbsd32_chmod	15

/* syscall: "chown" ret: "int" args: "const netbsd32_charp" "uid_t" "gid_t" */
#define	SVR4_32_SYS_chown	16

/* syscall: "break" ret: "int" args: "netbsd32_caddr_t" */
#define	SVR4_32_SYS_break	17

/* syscall: "stat" ret: "int" args: "const netbsd32_charp" "svr4_32_statp" */
#define	SVR4_32_SYS_stat	18

/* syscall: "compat_43_netbsd32_olseek" ret: "netbsd32_long" args: "int" "netbsd32_long" "int" */
#define	SVR4_32_SYS_compat_43_netbsd32_olseek	19

/* syscall: "getpid" ret: "pid_t" args: */
#define	SVR4_32_SYS_getpid	20

/* syscall: "netbsd32_setuid" ret: "int" args: "uid_t" */
#define	SVR4_32_SYS_netbsd32_setuid	23

/* syscall: "getuid_with_euid" ret: "uid_t" args: */
#define	SVR4_32_SYS_getuid_with_euid	24

/* syscall: "alarm" ret: "int" args: "unsigned" */
#define	SVR4_32_SYS_alarm	27

/* syscall: "fstat" ret: "int" args: "int" "svr4_32_statp" */
#define	SVR4_32_SYS_fstat	28

/* syscall: "pause" ret: "int" args: */
#define	SVR4_32_SYS_pause	29

/* syscall: "utime" ret: "int" args: "const netbsd32_charp" "svr4_32_utimbufp" */
#define	SVR4_32_SYS_utime	30

/* syscall: "access" ret: "int" args: "const netbsd32_charp" "int" */
#define	SVR4_32_SYS_access	33

/* syscall: "nice" ret: "int" args: "int" */
#define	SVR4_32_SYS_nice	34

/* syscall: "sync" ret: "int" args: */
#define	SVR4_32_SYS_sync	36

/* syscall: "kill" ret: "int" args: "int" "int" */
#define	SVR4_32_SYS_kill	37

/* syscall: "pgrpsys" ret: "int" args: "int" "int" "int" */
#define	SVR4_32_SYS_pgrpsys	39

/* syscall: "netbsd32_dup" ret: "int" args: "u_int" */
#define	SVR4_32_SYS_netbsd32_dup	41

/* syscall: "pipe" ret: "int" args: */
#define	SVR4_32_SYS_pipe	42

/* syscall: "times" ret: "int" args: "svr4_32_tms_tp" */
#define	SVR4_32_SYS_times	43

/* syscall: "netbsd32_setgid" ret: "int" args: "gid_t" */
#define	SVR4_32_SYS_netbsd32_setgid	46

/* syscall: "getgid_with_egid" ret: "gid_t" args: */
#define	SVR4_32_SYS_getgid_with_egid	47

/* syscall: "signal" ret: "int" args: "int" "svr4_sig_t" */
#define	SVR4_32_SYS_signal	48

#ifdef SYSVMSG
/* syscall: "msgsys" ret: "int" args: "int" "int" "int" "int" "int" */
#define	SVR4_32_SYS_msgsys	49

#else
#endif
/* syscall: "sysarch" ret: "int" args: "int" "netbsd32_voidp" */
#define	SVR4_32_SYS_sysarch	50

#ifdef SYSVSHM
/* syscall: "shmsys" ret: "int" args: "int" "int" "int" "int" */
#define	SVR4_32_SYS_shmsys	52

#else
#endif
#ifdef SYSVSEM
/* syscall: "semsys" ret: "int" args: "int" "int" "int" "int" "int" */
#define	SVR4_32_SYS_semsys	53

#else
#endif
/* syscall: "ioctl" ret: "int" args: "int" "netbsd32_u_long" "netbsd32_caddr_t" */
#define	SVR4_32_SYS_ioctl	54

/* syscall: "utssys" ret: "int" args: "netbsd32_voidp" "netbsd32_voidp" "int" "netbsd32_voidp" */
#define	SVR4_32_SYS_utssys	57

/* syscall: "netbsd32_fsync" ret: "int" args: "int" */
#define	SVR4_32_SYS_netbsd32_fsync	58

/* syscall: "netbsd32_execve" ret: "int" args: "const netbsd32_charp" "netbsd32_charpp" "netbsd32_charpp" */
#define	SVR4_32_SYS_netbsd32_execve	59

/* syscall: "netbsd32_umask" ret: "int" args: "int" */
#define	SVR4_32_SYS_netbsd32_umask	60

/* syscall: "netbsd32_chroot" ret: "int" args: "const netbsd32_charp" */
#define	SVR4_32_SYS_netbsd32_chroot	61

/* syscall: "fcntl" ret: "int" args: "int" "int" "netbsd32_charp" */
#define	SVR4_32_SYS_fcntl	62

/* syscall: "ulimit" ret: "netbsd32_long" args: "int" "netbsd32_long" */
#define	SVR4_32_SYS_ulimit	63

				/* 70 is obsolete advfs */
				/* 71 is obsolete unadvfs */
				/* 72 is obsolete rmount */
				/* 73 is obsolete rumount */
				/* 74 is obsolete rfstart */
				/* 75 is obsolete sigret */
				/* 76 is obsolete rdebug */
				/* 77 is obsolete rfstop */
/* syscall: "netbsd32_rmdir" ret: "int" args: "const netbsd32_charp" */
#define	SVR4_32_SYS_netbsd32_rmdir	79

/* syscall: "netbsd32_mkdir" ret: "int" args: "const netbsd32_charp" "mode_t" */
#define	SVR4_32_SYS_netbsd32_mkdir	80

/* syscall: "getdents" ret: "int" args: "int" "netbsd32_charp" "int" */
#define	SVR4_32_SYS_getdents	81

				/* 82 is obsolete libattach */
				/* 83 is obsolete libdetach */
/* syscall: "getmsg" ret: "int" args: "int" "svr4_32_strbuf_tp" "svr4_32_strbuf_tp" "netbsd32_intp" */
#define	SVR4_32_SYS_getmsg	85

/* syscall: "putmsg" ret: "int" args: "int" "svr4_32_strbuf_tp" "svr4_32_strbuf_tp" "int" */
#define	SVR4_32_SYS_putmsg	86

/* syscall: "netbsd32_poll" ret: "int" args: "netbsd32_pollfdp_t" "u_int" "int" */
#define	SVR4_32_SYS_netbsd32_poll	87

/* syscall: "lstat" ret: "int" args: "const netbsd32_charp" "svr4_32_stat_tp" */
#define	SVR4_32_SYS_lstat	88

/* syscall: "netbsd32_symlink" ret: "int" args: "const netbsd32_charp" "const netbsd32_charp" */
#define	SVR4_32_SYS_netbsd32_symlink	89

/* syscall: "netbsd32_readlink" ret: "int" args: "const netbsd32_charp" "netbsd32_charp" "netbsd32_size_t" */
#define	SVR4_32_SYS_netbsd32_readlink	90

/* syscall: "netbsd32_getgroups" ret: "int" args: "int" "netbsd32_gid_tp" */
#define	SVR4_32_SYS_netbsd32_getgroups	91

/* syscall: "netbsd32_setgroups" ret: "int" args: "int" "const netbsd32_gid_tp" */
#define	SVR4_32_SYS_netbsd32_setgroups	92

/* syscall: "netbsd32_fchmod" ret: "int" args: "int" "int" */
#define	SVR4_32_SYS_netbsd32_fchmod	93

/* syscall: "fchown" ret: "int" args: "int" "int" "int" */
#define	SVR4_32_SYS_fchown	94

/* syscall: "sigprocmask" ret: "int" args: "int" "const svr4_32_sigset_tp" "svr4_32_sigset_tp" */
#define	SVR4_32_SYS_sigprocmask	95

/* syscall: "sigsuspend" ret: "int" args: "const svr4_32_sigset_tp" */
#define	SVR4_32_SYS_sigsuspend	96

/* syscall: "sigaltstack" ret: "int" args: "const svr4_32_sigaltstack_tp" "svr4_32_sigaltstack_tp" */
#define	SVR4_32_SYS_sigaltstack	97

/* syscall: "sigaction" ret: "int" args: "int" "const svr4_32_sigaction_tp" "svr4_32_sigaction_tp" */
#define	SVR4_32_SYS_sigaction	98

/* syscall: "sigpending" ret: "int" args: "int" "svr4_32_sigset_tp" */
#define	SVR4_32_SYS_sigpending	99

/* syscall: "context" ret: "int" args: "int" "svr4_32_ucontext_tp" */
#define	SVR4_32_SYS_context	100

/* syscall: "statvfs" ret: "int" args: "const netbsd32_charp" "svr4_32_statvfs_tp" */
#define	SVR4_32_SYS_statvfs	103

/* syscall: "fstatvfs" ret: "int" args: "int" "svr4_32_statvfs_tp" */
#define	SVR4_32_SYS_fstatvfs	104

/* syscall: "waitsys" ret: "int" args: "int" "int" "svr4_32_siginfo_tp" "int" */
#define	SVR4_32_SYS_waitsys	107

/* syscall: "hrtsys" ret: "int" args: "int" "int" "int" "netbsd32_voidp" "netbsd32_voidp" */
#define	SVR4_32_SYS_hrtsys	109

/* syscall: "pathconf" ret: "int" args: "const netbsd32_charp" "int" */
#define	SVR4_32_SYS_pathconf	113

/* syscall: "mmap" ret: "netbsd32_voidp" args: "netbsd32_voidp" "svr4_32_size_t" "int" "int" "int" "svr4_32_off_t" */
#define	SVR4_32_SYS_mmap	115

/* syscall: "netbsd32_mprotect" ret: "int" args: "netbsd32_voidp" "netbsd32_size_t" "int" */
#define	SVR4_32_SYS_netbsd32_mprotect	116

/* syscall: "netbsd32_munmap" ret: "int" args: "netbsd32_voidp" "netbsd32_size_t" */
#define	SVR4_32_SYS_netbsd32_munmap	117

/* syscall: "fpathconf" ret: "int" args: "int" "int" */
#define	SVR4_32_SYS_fpathconf	118

/* syscall: "vfork" ret: "int" args: */
#define	SVR4_32_SYS_vfork	119

/* syscall: "netbsd32_fchdir" ret: "int" args: "int" */
#define	SVR4_32_SYS_netbsd32_fchdir	120

/* syscall: "netbsd32_readv" ret: "int" args: "int" "const netbsd32_iovecp_t" "int" */
#define	SVR4_32_SYS_netbsd32_readv	121

/* syscall: "netbsd32_writev" ret: "int" args: "int" "const netbsd32_iovecp_t" "int" */
#define	SVR4_32_SYS_netbsd32_writev	122

/* syscall: "xstat" ret: "int" args: "int" "const netbsd32_charp" "svr4_32_xstat_tp" */
#define	SVR4_32_SYS_xstat	123

/* syscall: "lxstat" ret: "int" args: "int" "const netbsd32_charp" "svr4_32_xstat_tp" */
#define	SVR4_32_SYS_lxstat	124

/* syscall: "fxstat" ret: "int" args: "int" "int" "svr4_32_xstat_tp" */
#define	SVR4_32_SYS_fxstat	125

/* syscall: "xmknod" ret: "int" args: "int" "netbsd32_charp" "svr4_32_mode_t" "svr4_dev_t" */
#define	SVR4_32_SYS_xmknod	126

/* syscall: "setrlimit" ret: "int" args: "int" "const svr4_32_rlimit_tp" */
#define	SVR4_32_SYS_setrlimit	128

/* syscall: "getrlimit" ret: "int" args: "int" "svr4_32_rlimit_tp" */
#define	SVR4_32_SYS_getrlimit	129

/* syscall: "lchown" ret: "int" args: "const netbsd32_charp" "uid_t" "gid_t" */
#define	SVR4_32_SYS_lchown	130

/* syscall: "memcntl" ret: "int" args: "netbsd32_voidp" "svr4_32_size_t" "int" "netbsd32_voidp" "int" "int" */
#define	SVR4_32_SYS_memcntl	131

/* syscall: "rename" ret: "int" args: "const netbsd32_charp" "const netbsd32_charp" */
#define	SVR4_32_SYS_rename	134

/* syscall: "uname" ret: "int" args: "svr4_32_utsnamep" "int" */
#define	SVR4_32_SYS_uname	135

/* syscall: "netbsd32_setegid" ret: "int" args: "gid_t" */
#define	SVR4_32_SYS_netbsd32_setegid	136

/* syscall: "sysconfig" ret: "int" args: "int" */
#define	SVR4_32_SYS_sysconfig	137

/* syscall: "netbsd32_adjtime" ret: "int" args: "const netbsd32_timevalp_t" "netbsd32_timevalp_t" */
#define	SVR4_32_SYS_netbsd32_adjtime	138

/* syscall: "systeminfo" ret: "netbsd32_long" args: "int" "netbsd32_charp" "netbsd32_long" */
#define	SVR4_32_SYS_systeminfo	139

/* syscall: "netbsd32_seteuid" ret: "int" args: "uid_t" */
#define	SVR4_32_SYS_netbsd32_seteuid	141

/* syscall: "fork1" ret: "int" args: */
#define	SVR4_32_SYS_fork1	143

/* syscall: "_lwp_info" ret: "int" args: "svr4_32_lwpinfop" */
#define	SVR4_32_SYS__lwp_info	145

/* syscall: "netbsd32_fchroot" ret: "int" args: "int" */
#define	SVR4_32_SYS_netbsd32_fchroot	153

/* syscall: "utimes" ret: "int" args: "const netbsd32_charp" "netbsd32_timevalp_t" */
#define	SVR4_32_SYS_utimes	154

/* syscall: "vhangup" ret: "int" args: */
#define	SVR4_32_SYS_vhangup	155

/* syscall: "gettimeofday" ret: "int" args: "netbsd32_timevalp_t" */
#define	SVR4_32_SYS_gettimeofday	156

/* syscall: "netbsd32_getitimer" ret: "int" args: "int" "netbsd32_itimervalp_t" */
#define	SVR4_32_SYS_netbsd32_getitimer	157

/* syscall: "netbsd32_setitimer" ret: "int" args: "int" "const netbsd32_itimervalp_t" "netbsd32_itimervalp_t" */
#define	SVR4_32_SYS_netbsd32_setitimer	158

/* syscall: "_lwp_create" ret: "int" args: "svr4_32_ucontext_tp" "netbsd32_u_long" "svr4_32_lwpid_tp" */
#define	SVR4_32_SYS__lwp_create	159

/* syscall: "_lwp_exit" ret: "int" args: */
#define	SVR4_32_SYS__lwp_exit	160

/* syscall: "_lwp_suspend" ret: "int" args: "svr4_lwpid_t" */
#define	SVR4_32_SYS__lwp_suspend	161

/* syscall: "_lwp_continue" ret: "int" args: "svr4_lwpid_t" */
#define	SVR4_32_SYS__lwp_continue	162

/* syscall: "_lwp_kill" ret: "int" args: "svr4_lwpid_t" "int" */
#define	SVR4_32_SYS__lwp_kill	163

/* syscall: "_lwp_self" ret: "svr4_lwpid_t" args: */
#define	SVR4_32_SYS__lwp_self	164

/* syscall: "_lwp_getprivate" ret: "netbsd32_voidp" args: */
#define	SVR4_32_SYS__lwp_getprivate	165

/* syscall: "_lwp_setprivate" ret: "int" args: "netbsd32_voidp" */
#define	SVR4_32_SYS__lwp_setprivate	166

/* syscall: "_lwp_wait" ret: "int" args: "svr4_lwpid_t" "svr4_32_lwpid_tp" */
#define	SVR4_32_SYS__lwp_wait	167

/* syscall: "pread" ret: "netbsd32_ssize_t" args: "int" "netbsd32_voidp" "netbsd32_size_t" "svr4_32_off_t" */
#define	SVR4_32_SYS_pread	173

/* syscall: "pwrite" ret: "netbsd32_ssize_t" args: "int" "const netbsd32_voidp" "netbsd32_size_t" "svr4_32_off_t" */
#define	SVR4_32_SYS_pwrite	174

/* syscall: "llseek" ret: "svr4_32_off64_t" args: "int" "netbsd32_long" "netbsd32_long" "int" */
#define	SVR4_32_SYS_llseek	175

/* syscall: "acl" ret: "int" args: "netbsd32_charp" "int" "int" "svr4_32_aclent_tp" */
#define	SVR4_32_SYS_acl	185

/* syscall: "auditsys" ret: "int" args: "int" "int" "int" "int" "int" "int" */
#define	SVR4_32_SYS_auditsys	186

/* syscall: "netbsd32_nanosleep" ret: "int" args: "const netbsd32_timespecp_t" "netbsd32_timespecp_t" */
#define	SVR4_32_SYS_netbsd32_nanosleep	199

/* syscall: "facl" ret: "int" args: "int" "int" "int" "svr4_32_aclent_tp" */
#define	SVR4_32_SYS_facl	200

/* syscall: "netbsd32_setreuid" ret: "int" args: "int" "int" */
#define	SVR4_32_SYS_netbsd32_setreuid	202

/* syscall: "netbsd32_setregid" ret: "int" args: "int" "int" */
#define	SVR4_32_SYS_netbsd32_setregid	203

/* syscall: "schedctl" ret: "int" args: "unsigned int" "int" "void **" */
#define	SVR4_32_SYS_schedctl	206

/* syscall: "resolvepath" ret: "int" args: "const netbsd32_charp" "netbsd32_charp" "netbsd32_size_t" */
#define	SVR4_32_SYS_resolvepath	209

/* syscall: "getdents64" ret: "int" args: "int" "svr4_32_dirent64_tp" "int" */
#define	SVR4_32_SYS_getdents64	213

/* syscall: "mmap64" ret: "netbsd32_voidp" args: "netbsd32_voidp" "svr4_32_size_t" "int" "int" "int" "svr4_32_off64_t" */
#define	SVR4_32_SYS_mmap64	214

/* syscall: "stat64" ret: "int" args: "const netbsd32_charp" "svr4_32_stat64_tp" */
#define	SVR4_32_SYS_stat64	215

/* syscall: "lstat64" ret: "int" args: "const netbsd32_charp" "svr4_32_stat64_tp" */
#define	SVR4_32_SYS_lstat64	216

/* syscall: "fstat64" ret: "int" args: "int" "svr4_32_stat64_tp" */
#define	SVR4_32_SYS_fstat64	217

/* syscall: "statvfs64" ret: "int" args: "const netbsd32_charp" "svr4_32_statvfs64_tp" */
#define	SVR4_32_SYS_statvfs64	218

/* syscall: "fstatvfs64" ret: "int" args: "int" "svr4_32_statvfs64_tp" */
#define	SVR4_32_SYS_fstatvfs64	219

/* syscall: "setrlimit64" ret: "int" args: "int" "const svr4_32_rlimit64_tp" */
#define	SVR4_32_SYS_setrlimit64	220

/* syscall: "getrlimit64" ret: "int" args: "int" "svr4_32_rlimit64_tp" */
#define	SVR4_32_SYS_getrlimit64	221

/* syscall: "pread64" ret: "netbsd32_ssize_t" args: "int" "netbsd32_voidp" "netbsd32_size_t" "svr4_32_off64_t" */
#define	SVR4_32_SYS_pread64	222

/* syscall: "pwrite64" ret: "netbsd32_ssize_t" args: "int" "const netbsd32_voidp" "netbsd32_size_t" "svr4_32_off64_t" */
#define	SVR4_32_SYS_pwrite64	223

/* syscall: "creat64" ret: "int" args: "netbsd32_charp" "int" */
#define	SVR4_32_SYS_creat64	224

/* syscall: "open64" ret: "int" args: "netbsd32_charp" "int" "int" */
#define	SVR4_32_SYS_open64	225

/* syscall: "socket" ret: "int" args: "int" "int" "int" */
#define	SVR4_32_SYS_socket	230

/* syscall: "netbsd32_socketpair" ret: "int" args: "int" "int" "int" "netbsd32_intp" */
#define	SVR4_32_SYS_netbsd32_socketpair	231

/* syscall: "netbsd32_bind" ret: "int" args: "int" "const netbsd32_sockaddrp_t" "int" */
#define	SVR4_32_SYS_netbsd32_bind	232

/* syscall: "netbsd32_listen" ret: "int" args: "int" "int" */
#define	SVR4_32_SYS_netbsd32_listen	233

/* syscall: "compat_43_netbsd32_oaccept" ret: "int" args: "int" "netbsd32_sockaddrp_t" "netbsd32_intp" */
#define	SVR4_32_SYS_compat_43_netbsd32_oaccept	234

/* syscall: "netbsd32_connect" ret: "int" args: "int" "const netbsd32_sockaddrp_t" "int" */
#define	SVR4_32_SYS_netbsd32_connect	235

/* syscall: "netbsd32_shutdown" ret: "int" args: "int" "int" */
#define	SVR4_32_SYS_netbsd32_shutdown	236

/* syscall: "compat_43_netbsd32_orecv" ret: "int" args: "int" "netbsd32_caddr_t" "int" "int" */
#define	SVR4_32_SYS_compat_43_netbsd32_orecv	237

/* syscall: "compat_43_netbsd32_orecvfrom" ret: "int" args: "int" "netbsd32_caddr_t" "netbsd32_size_t" "int" "netbsd32_caddr_t" "netbsd32_intp" */
#define	SVR4_32_SYS_compat_43_netbsd32_orecvfrom	238

/* syscall: "compat_43_netbsd32_orecvmsg" ret: "int" args: "int" "netbsd32_omsghdrp_t" "int" */
#define	SVR4_32_SYS_compat_43_netbsd32_orecvmsg	239

/* syscall: "compat_43_netbsd32_osend" ret: "int" args: "int" "netbsd32_caddr_t" "int" "int" */
#define	SVR4_32_SYS_compat_43_netbsd32_osend	240

/* syscall: "compat_43_netbsd32_osendmsg" ret: "int" args: "int" "netbsd32_caddr_t" "int" */
#define	SVR4_32_SYS_compat_43_netbsd32_osendmsg	241

/* syscall: "netbsd32_sendto" ret: "netbsd32_ssize_t" args: "int" "const netbsd32_voidp" "netbsd32_size_t" "int" "const netbsd32_sockaddrp_t" "int" */
#define	SVR4_32_SYS_netbsd32_sendto	242

/* syscall: "compat_43_netbsd32_ogetpeername" ret: "int" args: "int" "netbsd32_caddr_t" "netbsd32_intp" */
#define	SVR4_32_SYS_compat_43_netbsd32_ogetpeername	243

/* syscall: "compat_43_netbsd32_ogetsockname" ret: "int" args: "int" "netbsd32_caddr_t" "netbsd32_intp" */
#define	SVR4_32_SYS_compat_43_netbsd32_ogetsockname	244

/* syscall: "netbsd32_getsockopt" ret: "int" args: "int" "int" "int" "netbsd32_voidp" "netbsd32_intp" */
#define	SVR4_32_SYS_netbsd32_getsockopt	245

/* syscall: "netbsd32_setsockopt" ret: "int" args: "int" "int" "int" "const netbsd32_voidp" "int" */
#define	SVR4_32_SYS_netbsd32_setsockopt	246

/* syscall: "netbsd32_ntp_gettime" ret: "int" args: "netbsd32_ntptimevalp_t" */
#define	SVR4_32_SYS_netbsd32_ntp_gettime	248

#if defined(NTP) || !defined(_KERNEL)
/* syscall: "netbsd32_ntp_adjtime" ret: "int" args: "netbsd32_timexp_t" */
#define	SVR4_32_SYS_netbsd32_ntp_adjtime	249

#else
				/* 249 is excluded ntp_adjtime */
#endif
#define	SVR4_32_SYS_MAXSYSCALL	256
#define	SVR4_32_SYS_NSYSENT	256
