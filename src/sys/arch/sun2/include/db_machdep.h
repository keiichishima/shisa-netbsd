/*	$NetBSD: db_machdep.h,v 1.5 2005/01/22 15:36:09 chs Exp $	*/

/*
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */

#ifndef	_SUN2_DB_MACHDEP_H_
#define	_SUN2_DB_MACHDEP_H_

/*
 * Machine-dependent defines for the DDB kernel debugger.
 */

#include <m68k/db_machdep.h>
#define DB_ELF_SYMBOLS
#define DB_ELFSIZE 32

/* This enables some code in db_command.c */
#define DB_MACHINE_COMMANDS

extern char	*esym;	/* end of symbols */

/* These are in db_memrw.c */
extern void db_read_bytes (db_addr_t, size_t, char *);
extern void db_write_bytes(db_addr_t, size_t, char *);

#endif	/* _SUN2_DB_MACHDEP_H_ */
