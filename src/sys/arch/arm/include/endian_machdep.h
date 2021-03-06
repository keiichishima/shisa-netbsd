/* $NetBSD: endian_machdep.h,v 1.6 2004/06/10 16:01:39 kleink Exp $ */

/* GCC predefines __ARMEB__ when building for big-endian ARM. */
#ifdef __ARMEB__
#define _BYTE_ORDER _BIG_ENDIAN
#else
#define _BYTE_ORDER _LITTLE_ENDIAN
#endif

#ifdef __GNUC__

#include <arm/byte_swap.h>

#if _BYTE_ORDER == _LITTLE_ENDIAN
#define	ntohl(x)	((uint32_t)__byte_swap_32((uint32_t)(x)))
#define	ntohs(x)	((uint16_t)__byte_swap_16((uint16_t)(x)))
#define	htonl(x)	((uint32_t)__byte_swap_32((uint32_t)(x)))
#define	htons(x)	((uint16_t)__byte_swap_16((uint16_t)(x)))
#endif

#endif
