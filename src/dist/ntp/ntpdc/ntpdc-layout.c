/*	$NetBSD: ntpdc-layout.c,v 1.2 2003/12/04 16:23:38 drochner Exp $	*/

/*
 * ntpdc-layout - print layout of NTP mode 7 request/response packets
 */

#include <stdio.h>
#include <stddef.h>

#include "ntpdc.h"
#include "ntp_stdlib.h"

#if defined(IMPL_XNTPD_OLD) && IMPL_XNTPD != 3
#error Unexpected IMPL_XNTPD
#endif

int
main(void)
{
#include "nl.c"

  return (EXIT_SUCCESS);
}
