/*	$NetBSD: lpc.c,v 1.16 2004/10/30 08:44:26 dsl Exp $	*/

/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#ifndef lint
__COPYRIGHT("@(#) Copyright (c) 1983, 1993\n\
	The Regents of the University of California.  All rights reserved.\n");
#if 0
static char sccsid[] = "@(#)lpc.c	8.3 (Berkeley) 4/28/95";
#else
__RCSID("$NetBSD: lpc.c,v 1.16 2004/10/30 08:44:26 dsl Exp $");
#endif
#endif /* not lint */

#include <sys/param.h>

#include <dirent.h>
#include <signal.h>
#include <setjmp.h>
#include <syslog.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <grp.h>
#include <err.h>
#include "lp.h"
#include "lpc.h"
#include "extern.h"

#ifndef LPR_OPER
#define LPR_OPER	"operator"	/* group name of lpr operators */
#endif

/*
 * lpc -- line printer control program
 */

#define MAX_CMDLINE	200
#define MAX_MARGV	20
int	fromatty;

char	cmdline[MAX_CMDLINE];
int	margc;
char	*margv[MAX_MARGV];
int	top;
uid_t	uid, euid;

jmp_buf	toplevel;

static void		 cmdscanner(int);
static struct cmd	*getcmd(char *);
static void		 intr(int);
static void		 makeargv(void);
static int		 ingroup(char *);
int			 main(int, char *p[]);

int
main(int argc, char *argv[])
{
	struct cmd *c;

	euid = geteuid();
	uid = getuid();
	seteuid(uid);
	name = argv[0];
	openlog("lpd", 0, LOG_LPR);

	if (--argc > 0) {
		c = getcmd(*++argv);
		if (c == (struct cmd *)-1) {
			printf("?Ambiguous command\n");
			exit(1);
		}
		if (c == 0) {
			printf("?Invalid command\n");
			exit(1);
		}
		if (c->c_priv && getuid() && ingroup(LPR_OPER) == 0) {
			printf("?Privileged command\n");
			exit(1);
		}
		(*c->c_handler)(argc, argv);
		exit(0);
	}
	fromatty = isatty(fileno(stdin));
	top = setjmp(toplevel) == 0;
	if (top)
		signal(SIGINT, intr);
	for (;;) {
		cmdscanner(top);
		top = 1;
	}
}

static void
intr(int signo)
{
	if (!fromatty)
		exit(0);
	longjmp(toplevel, 1);
}

/*
 * Command parser.
 */
static void
cmdscanner(int top)
{
	struct cmd *c;

	if (!top)
		putchar('\n');
	for (;;) {
		if (fromatty) {
			printf("lpc> ");
			fflush(stdout);
		}
		if (fgets(cmdline, MAX_CMDLINE, stdin) == 0)
			quit(0, NULL);
		makeargv();
		if (margc == 0)
			break;
		c = getcmd(margv[0]);
		if (c == (struct cmd *)-1) {
			printf("?Ambiguous command\n");
			continue;
		}
		if (c == 0) {
			printf("?Invalid command\n");
			continue;
		}
		if (c->c_priv && getuid() && ingroup(LPR_OPER) == 0) {
			printf("?Privileged command\n");
			continue;
		}
		(*c->c_handler)(margc, margv);
	}
	longjmp(toplevel, 0);
}

static struct cmd *
getcmd(char *name)
{
	char *p, *q;
	struct cmd *c, *found;
	int nmatches, longest;

	longest = 0;
	nmatches = 0;
	found = 0;
	for (c = cmdtab; (p = c->c_name) != NULL; c++) {
		for (q = name; *q == *p++; q++)
			if (*q == 0)		/* exact match? */
				return(c);
		if (!*q) {			/* the name was a prefix */
			if (q - name > longest) {
				longest = q - name;
				nmatches = 1;
				found = c;
			} else if (q - name == longest)
				nmatches++;
		}
	}
	if (nmatches > 1)
		return((struct cmd *)-1);
	return(found);
}

/*
 * Slice a string up into argc/argv.
 */
static void
makeargv(void)
{
	char *cp;
	char **argp = margv;
	int n = 0;

	margc = 0;
	for (cp = cmdline; *cp && (cp - cmdline) < sizeof(cmdline) &&
	    n < MAX_MARGV; n++) {
		while (isspace((unsigned char)*cp))
			cp++;
		if (*cp == '\0')
			break;
		*argp++ = cp;
		margc += 1;
		while (*cp != '\0' && !isspace((unsigned char)*cp))
			cp++;
		if (*cp == '\0')
			break;
		*cp++ = '\0';
	}
	*argp++ = 0;
}

#define HELPINDENT (sizeof ("directory"))

/*
 * Help command.
 */
void
help(int argc, char *argv[])
{
	struct cmd *c;

	if (argc == 1) {
		int i, j, w;
		int columns, width = 0, lines;

		printf("Commands may be abbreviated.  Commands are:\n\n");
		for (c = cmdtab; c->c_name; c++) {
			int len = strlen(c->c_name);

			if (len > width)
				width = len;
		}
		width = (width + 8) &~ 7;
		columns = 80 / width;
		if (columns == 0)
			columns = 1;
		lines = (NCMDS + columns - 1) / columns;
		for (i = 0; i < lines; i++) {
			for (j = 0; j < columns; j++) {
				c = cmdtab + j * lines + i;
				if (c->c_name)
					printf("%s", c->c_name);
				if (c + lines >= &cmdtab[NCMDS]) {
					printf("\n");
					break;
				}
				w = strlen(c->c_name);
				while (w < width) {
					w = (w + 8) &~ 7;
					putchar('\t');
				}
			}
		}
		return;
	}
	while (--argc > 0) {
		char *arg;
		arg = *++argv;
		c = getcmd(arg);
		if (c == (struct cmd *)-1)
			printf("?Ambiguous help command %s\n", arg);
		else if (c == (struct cmd *)0)
			printf("?Invalid help command %s\n", arg);
		else
			printf("%-*s\t%s\n", (int)HELPINDENT,
				c->c_name, c->c_help);
	}
}

/*
 * return non-zero if the user is a member of the given group
 */
static int
ingroup(char *grname)
{
	static struct group *gptr=NULL;
	static gid_t groups[NGROUPS];
	static int ngroups;
	gid_t gid;
	int i;

	if (gptr == NULL) {
		if ((gptr = getgrnam(grname)) == NULL) {
			warnx("Warning: unknown group `%s'",
				grname);
			return(0);
		}
		ngroups = getgroups(NGROUPS, groups);
		if (ngroups < 0)
			err(1, "getgroups");
	}
	gid = gptr->gr_gid;
	for (i = 0; i < ngroups; i++)
		if (gid == groups[i])
			return(1);
	return(0);
}
