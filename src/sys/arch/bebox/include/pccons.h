/*	$NetBSD: pccons.h,v 1.2 2003/10/24 18:21:52 matt Exp $	*/

/*
 * pccons.h -- pccons ioctl definitions
 */

#ifndef _PCCONS_H_
#define _PCCONS_H_

#include <sys/ioctl.h>

#define CONSOLE_X_MODE_ON		_IO('t',121)
#define CONSOLE_X_MODE_OFF		_IO('t',122)
#define CONSOLE_X_BELL			_IOW('t',123,int[2])
#define CONSOLE_SET_TYPEMATIC_RATE	_IOW('t',124,u_char)

#ifdef _KERNEL
int	pccnattach(void);
#endif

#endif /* _PCCONS_H_ */
