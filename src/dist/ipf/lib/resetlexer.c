/*	$NetBSD: resetlexer.c,v 1.1.1.1 2004/03/28 08:56:21 martti Exp $	*/

#include "ipf.h"

long	string_start = -1;
long	string_end = -1;
char	*string_val = NULL;
long	pos = 0;


void resetlexer()
{
	string_start = -1;
	string_end = -1;
	string_val = NULL;
	pos = 0;
}
