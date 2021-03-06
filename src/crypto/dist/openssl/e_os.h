/* e_os.h */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
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
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#ifndef HEADER_E_OS_H
#define HEADER_E_OS_H

#include <openssl/opensslconf.h>

#include <openssl/e_os2.h>
/* <openssl/e_os2.h> contains what we can justify to make visible
 * to the outside; this file e_os.h is not part of the exported
 * interface. */

#ifdef  __cplusplus
extern "C" {
#endif

/* Used to checking reference counts, most while doing perl5 stuff :-) */
#ifdef REF_PRINT
#undef REF_PRINT
#define REF_PRINT(a,b)	fprintf(stderr,"%08X:%4d:%s\n",(int)b,b->references,a)
#endif

#ifndef DEVRANDOM
/* set this to a comma-separated list of 'random' device files to try out.
 * My default, we will try to read at least one of these files */
#define DEVRANDOM "/dev/urandom","/dev/random","/dev/srandom"
#endif
#ifndef DEVRANDOM_EGD
/* set this to a comma-seperated list of 'egd' sockets to try out. These
 * sockets will be tried in the order listed in case accessing the device files
 * listed in DEVRANDOM did not return enough entropy. */
#define DEVRANDOM_EGD "/var/run/egd-pool","/dev/egd-pool","/etc/egd-pool","/etc/entropy"
#endif

  

/********************************************************************
 The Microsoft section
 ********************************************************************/
/* The following is used becaue of the small stack in some
 * Microsoft operating systems */
#  define MS_STATIC


#if defined(MSDOS) && !defined(GETPID_IS_MEANINGLESS)
#  define GETPID_IS_MEANINGLESS
#endif

#define get_last_sys_error()	errno
#define clear_sys_error()	errno=0

#define get_last_socket_error()	errno
#define clear_socket_error()	errno=0
#define ioctlsocket(a,b,c)	ioctl(a,b,c)
#define closesocket(s)		close(s)
#define readsocket(s,b,n)	read((s),(b),(n))
#define writesocket(s,b,n)	write((s),(b),(n))

#  define MS_CALLBACK
#  define MS_FAR



     /* !defined VMS */
#    ifdef OPENSSL_UNISTD
#      include OPENSSL_UNISTD
#    else
#      include <unistd.h>
#    endif
#    ifndef NO_SYS_TYPES_H
#      include <sys/types.h>
#    endif

#    define OPENSSL_CONF	"openssl.cnf"
#    define SSLEAY_CONF		OPENSSL_CONF
#    define RFILE		".rnd"
#    define LIST_SEPARATOR_CHAR ':'
#    define NUL_DEV		"/dev/null"
#    define EXIT(n)		exit(n)

#  define SSLeay_getpid()	getpid()



/*************/

#ifdef USE_SOCKETS

#    ifndef NO_SYS_PARAM_H
#      include <sys/param.h>
#    endif

#    include <netdb.h>
#      include <sys/socket.h>
#      ifdef FILIO_H
#        include <sys/filio.h> /* Added for FIONBIO under unixware */
#      endif
#      include <netinet/in.h>
#      include <arpa/inet.h>




#        include <sys/ioctl.h>


#    define SSLeay_Read(a,b,c)     read((a),(b),(c))
#    define SSLeay_Write(a,b,c)    write((a),(b),(c))
#    define SHUTDOWN(fd)    { shutdown((fd),0); closesocket((fd)); }
#    define SHUTDOWN2(fd)   { shutdown((fd),2); closesocket((fd)); }
#    ifndef INVALID_SOCKET
#    define INVALID_SOCKET	(-1)
#    endif /* INVALID_SOCKET */
#endif



#ifndef OPENSSL_EXIT
# if defined(MONOLITH) && !defined(OPENSSL_C)
#  define OPENSSL_EXIT(n) return(n)
# else
#  define OPENSSL_EXIT(n) do { EXIT(n); return(n); } while(0)
# endif
#endif

/***********************************************/

/* do we need to do this for getenv.
 * Just define getenv for use under windows */

#define Getenv getenv

#define DG_GCC_BUG	/* gcc < 2.6.3 on DGUX */



/* vxworks */
/* end vxworks */

#ifdef  __cplusplus
}
#endif

#endif

