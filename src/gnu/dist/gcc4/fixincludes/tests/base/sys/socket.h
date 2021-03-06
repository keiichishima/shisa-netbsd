/*  DO NOT EDIT THIS FILE.

    It has been auto-edited by fixincludes from:

	"fixinc/tests/inc/sys/socket.h"

    This had to be done to correct non-standard usages in the
    original, manufacturer supplied header file.  */



#if defined( IRIX_SOCKLEN_T_CHECK )
#define _SOCKLEN_T
#if _NO_XOPEN4 && _NO_XOPEN5
typedef int socklen_t;
#else
typedef u_int32_t socklen_t;
#endif /* _NO_XOPEN4 && _NO_XOPEN5 */
#endif  /* IRIX_SOCKLEN_T_CHECK */


#if defined( SOLARIS_SOCKET_CHECK )
#ident   "@(#)socket.h   1.30    97/01/20 SMI"
extern int recv(int, void *, int, int);
extern int send(int, const void *, int, int);
#endif  /* SOLARIS_SOCKET_CHECK */
