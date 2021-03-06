/*	$NetBSD: curses_private.h,v 1.37 2005/02/18 22:16:27 dsl Exp $	*/

/*-
 * Copyright (c) 1998-2000 Brett Lymn
 *                         (blymn@baea.com.au, brett_lymn@yahoo.com.au)
 * All rights reserved.
 *
 * This code has been donated to The NetBSD Foundation by the Author.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 */

#include <termios.h>

/* Private structure definitions for curses. */

/* Termcap capabilities. */
extern char	__tc_am, __tc_bs, __tc_cc, __tc_da, __tc_eo,
		__tc_hc, __tc_hl, __tc_in, __tc_mi, __tc_ms,
		__tc_nc, __tc_ns, __tc_os, __tc_ul, __tc_ut,
		__tc_xb, __tc_xn, __tc_xt, __tc_xs, __tc_xx;
extern char	__CA;
extern int	__tc_pa, __tc_Co, __tc_NC;
extern char	*__tc_ac, *__tc_AB, *__tc_ae, *__tc_AF, *__tc_AL,
		*__tc_al, *__tc_as, *__tc_bc, *__tc_bl, *__tc_bt,
		*__tc_cd, *__tc_ce, *__tc_cl, *__tc_cm, *__tc_cr,
		*__tc_cs, *__tc_dc, *__tc_DL, *__tc_dl, *__tc_dm,
		*__tc_DO, *__tc_do, *__tc_eA, *__tc_ed, *__tc_ei,
		*__tc_ho, *__tc_Ic, *__tc_ic, *__tc_im, *__tc_Ip,
		*__tc_ip, *__tc_k0, *__tc_k1, *__tc_k2, *__tc_k3,
		*__tc_k4, *__tc_k5, *__tc_k6, *__tc_k7, *__tc_k8,
		*__tc_k9, *__tc_kd, *__tc_ke, *__tc_kh, *__tc_kl,
		*__tc_kr, *__tc_ks, *__tc_ku, *__tc_LE, *__tc_ll,
		*__tc_ma, *__tc_mb, *__tc_md, *__tc_me, *__tc_mh,
		*__tc_mk, *__tc_mm, *__tc_mo, *__tc_mp, *__tc_mr,
		*__tc_nd, *__tc_nl, *__tc_oc, *__tc_op,
		*__tc_rc, *__tc_RI, *__tc_Sb, *__tc_sc, *__tc_se,
		*__tc_SF, *__tc_Sf, *__tc_sf, *__tc_so, *__tc_sp,
		*__tc_SR, *__tc_sr, *__tc_ta, *__tc_te, *__tc_ti,
		*__tc_uc, *__tc_ue, *__tc_UP, *__tc_up, *__tc_us,
		*__tc_vb, *__tc_ve, *__tc_vi, *__tc_vs;

/*
 * A window an array of __LINE structures pointed to by the 'lines' pointer.
 * A line is an array of __LDATA structures pointed to by the 'line' pointer.
 *
 * IMPORTANT: the __LDATA structure must NOT induce any padding, so if new
 * fields are added -- padding fields with *constant values* should ensure
 * that the compiler will not generate any padding when storing an array of
 *  __LDATA structures.  This is to enable consistent use of memcmp, and memcpy
 * for comparing and copying arrays.
 */

struct __ldata {
	wchar_t	ch;			/* Character */
	attr_t	attr;			/* Attributes */
	wchar_t	bch;			/* Background character */
	attr_t	battr;			/* Background attributes */
};

#define __LDATASIZE	(sizeof(__LDATA))

struct __line {
#ifdef DEBUG
#define SENTINEL_VALUE 0xaac0ffee
	
	unsigned int sentinel;          /* try to catch line overflows */
#endif
#define	__ISDIRTY	0x01		/* Line is dirty. */
#define __ISPASTEOL	0x02		/* Cursor is past end of line */
	unsigned int flags;
	unsigned int hash;		/* Hash value for the line. */
	int *firstchp, *lastchp;	/* First and last chngd columns ptrs */
	int firstch, lastch;		/* First and last changed columns. */
	__LDATA *line;			/* Pointer to the line text. */
};

struct __window {		/* Window structure. */
	struct __window	*nextp, *orig;	/* Subwindows list and parent. */
	int begy, begx;			/* Window home. */
	int cury, curx;			/* Current x, y coordinates. */
	int maxy, maxx;			/* Maximum values for curx, cury. */
	int reqy, reqx;			/* Size requested when created */
	int ch_off;			/* x offset for firstch/lastch. */
	__LINE **lines;			/* Array of pointers to the lines */
	__LINE  *lspace;		/* line space (for cleanup) */
	__LDATA *wspace;		/* window space (for cleanup) */

#define	__ENDLINE	0x00000001	/* End of screen. */
#define	__FLUSH		0x00000002	/* Fflush(stdout) after refresh. */
#define	__FULLWIN	0x00000004	/* Window is a screen. */
#define	__IDLINE	0x00000008	/* Insert/delete sequences. */
#define	__SCROLLWIN	0x00000010	/* Last char will scroll window. */
#define	__SCROLLOK	0x00000020	/* Scrolling ok. */
#define	__CLEAROK	0x00000040	/* Clear on next refresh. */
#define	__LEAVEOK	0x00000100	/* If cursor left */
#define	__KEYPAD	0x00010000	/* If interpreting keypad codes */
#define	__NOTIMEOUT	0x00020000	/* Wait indefinitely for func keys */
#define __IDCHAR	0x00040000	/* insert/delete char sequences */
#define __ISPAD		0x00080000	/* "window" is a pad */
	unsigned int flags;
	int	delay;			/* delay for getch() */
	attr_t	wattr;			/* Character attributes */
	wchar_t	bch;			/* Background character */
	attr_t	battr;			/* Background attributes */
	int	scr_t, scr_b;		/* Scrolling region top, bottom */
	SCREEN	*screen;		/* Screen for this window */
	int	pbegy, pbegx,
		sbegy, sbegx,
		smaxy, smaxx;		/* Saved prefresh() values */
};

/* Set of attributes unset by 'me' - 'mb', 'md', 'mh', 'mk', 'mp' and 'mr'. */
#define	__TERMATTR \
	(__REVERSE | __BLINK | __DIM | __BOLD | __BLANK | __PROTECT)

struct __winlist {
	struct __window		*winp;	/* The window. */
	struct __winlist	*nextp;	/* Next window. */
};

struct __color {
	short	num;
	short	red;
	short	green;
	short	blue;
	int	flags;
};

/* List of colour pairs */
struct __pair {
	short	fore;
	short	back;
	int	flags;
};

/* Maximum colours */
#define	MAX_COLORS	64
/* Maximum colour pairs - determined by number of colour bits in attr_t */
#define	MAX_PAIRS	PAIR_NUMBER(__COLOR)

typedef struct keymap keymap_t;

/* this is the encapsulation of the terminal definition, one for
 * each terminal that curses talks to.
 */
struct __screen {
	FILE    *infd, *outfd;  /* input and output file descriptors */
	WINDOW	*curscr;	/* Current screen. */
	WINDOW	*stdscr;	/* Standard screen. */
	WINDOW	*__virtscr;	/* Virtual screen (for doupdate()). */
	int      curwin;        /* current window for refresh */
	int      lx, ly;        /* loop parameters for refresh */
	int	 COLS;		/* Columns on the screen. */
	int	 LINES;		/* Lines on the screen. */
	int	 COLORS;	/* Maximum colors on the screen */
	int	 COLOR_PAIRS;	/* Maximum color pairs on the screen */
	int	 My_term;	/* Use Def_term regardless. */
	char	 GT;		/* Gtty indicates tabs. */
	char	 NONL;		/* Term can't hack LF doing a CR. */
	char	 UPPERCASE;	/* Terminal is uppercase only. */

        /* BEWARE!!! The order of the following terminal capabilities */
        /* (tc_foo) is important - do not update without fixing the zap */
        /* function in setterm.c */
	char	tc_am, tc_bs, tc_cc, tc_da, tc_eo, tc_hc, tc_hl, tc_in, tc_mi,
		tc_ms, tc_nc, tc_ns, tc_os, tc_ul, tc_ut, tc_xb, tc_xn, tc_xt,
		tc_xs, tc_xx;
	int     flag_count;
	int	tc_pa, tc_Co, tc_NC;
	int     int_count;
	char	*tc_AB, *tc_ac, *tc_ae, *tc_AF, *tc_AL,
		*tc_al, *tc_as, *tc_bc, *tc_bl, *tc_bt,
		*tc_cd, *tc_ce, *tc_cl, *tc_cm, *tc_cr,
		*tc_cs, *tc_dc, *tc_DL, *tc_dl, *tc_dm,
		*tc_DO, *tc_do, *tc_eA, *tc_ed, *tc_ei,
		*tc_ho, *tc_Ic, *tc_ic, *tc_im, *tc_Ip,
		*tc_ip, *tc_k0, *tc_k1, *tc_k2, *tc_k3,
		*tc_k4, *tc_k5, *tc_k6, *tc_k7, *tc_k8,
		*tc_k9, *tc_kd, *tc_ke, *tc_kh, *tc_kl,
		*tc_kr, *tc_ks, *tc_ku, *tc_LE, *tc_ll,
		*tc_ma, *tc_mb, *tc_md, *tc_me, *tc_mh,
		*tc_mk, *tc_mm, *tc_mo, *tc_mp, *tc_mr,
		*tc_nd, *tc_nl, *tc_oc, *tc_op, *tc_pc,
		*tc_rc, *tc_RI, *tc_Sb, *tc_sc, *tc_se,
		*tc_SF, *tc_Sf, *tc_sf, *tc_so, *tc_sp,
		*tc_SR, *tc_sr, *tc_ta, *tc_te, *tc_ti,
		*tc_uc, *tc_ue, *tc_UP, *tc_up, *tc_us,
		*tc_vb, *tc_ve, *tc_vi, *tc_vs;
	char CA;
	int str_count;
	chtype acs_char[NUM_ACS];
	struct __color colours[MAX_COLORS];
	struct __pair  colour_pairs[MAX_PAIRS];
	attr_t	nca;

/* Style of colour manipulation */
#define COLOR_NONE	0
#define COLOR_ANSI	1	/* ANSI/DEC-style colour manipulation */
#define COLOR_HP	2	/* HP-style colour manipulation */
#define COLOR_TEK	3	/* Tektronix-style colour manipulation */
#define COLOR_OTHER	4	/* None of the others but can set fore/back */
	int color_type;

	attr_t mask_op;
	attr_t mask_me;
	attr_t mask_ue;
	attr_t mask_se;
	struct tinfo *cursesi_genbuf;
	int old_mode; /* old cursor visibility state for terminal */
	keymap_t *base_keymap;
	int echoit;
	int pfast;
	int rawmode;
	int nl;
	int noqch;
	int clearok;
	int useraw;
	struct __winlist *winlistp;
	struct   termios cbreakt, rawt, *curt, save_termios;
	struct termios orig_termios, baset, savedtty;
	int ovmin;
	int ovtime;
	char *stdbuf;
	unsigned int len;
	int meta_state;
	char pad_char;
	char ttytype[128];
	int endwin;
	int notty;
	int half_delay;
	int resized;
};


extern char	 __GT;			/* Gtty indicates tabs. */
extern char	 __NONL;		/* Term can't hack LF doing a CR. */
extern char	 __UPPERCASE;		/* Terminal is uppercase only. */
extern int	 My_term;		/* Use Def_term regardless. */
extern const char	*Def_term;	/* Default terminal type. */
extern SCREEN   *_cursesi_screen;       /* The current screen in use */

/* Private functions. */
#ifdef DEBUG
void	 __CTRACE(const char *, ...) __attribute__((__format__(__printf__, 1, 0)));
#endif
void     __cputchar_args(char, void *);
void     _cursesi_free_keymap(keymap_t *);
int      _cursesi_gettmode(SCREEN *);
void     _cursesi_reset_acs(SCREEN *);
void     _cursesi_resetterm(SCREEN *);
int      _cursesi_setterm(char *, SCREEN *);
int	 __delay(void);
u_int	 __hash_more(const void *, size_t, u_int);
#define	__hash(s, len)	__hash_more((s), (len), 0u)
void	 __id_subwins(WINDOW *);
void	 __init_getch(SCREEN *);
void	 __init_acs(SCREEN *);
char	*__longname(char *, char *);	/* Original BSD version */
int	 __mvcur(int, int, int, int, int);
WINDOW  *__newwin(SCREEN *, int, int, int, int, int);
int	 __nodelay(void);
int	 __notimeout(void);
char	*__parse_cap(const char *, ...);
void	 __restartwin(void);
void	 __restore_colors(void);
void     __restore_cursor_vis(void);
void     __restore_meta_state(void);
void	 __restore_termios(void);
void	 __restore_stophandler(void);
void	 __restore_winchhandler(void);
void	 __save_termios(void);
void	 __set_color(WINDOW *win, attr_t attr);
void	 __set_stophandler(void);
void	 __set_winchhandler(void);
void	 __set_subwin(WINDOW *, WINDOW *);
void	 __startwin(SCREEN *);
void	 __stop_signal_handler(int);
int	 __stopwin(void);
void	 __swflags(WINDOW *);
int	 __timeout(int);
int	 __touchline(WINDOW *, int, int, int);
int	 __touchwin(WINDOW *);
char	*__tscroll(const char *, int, int);
void	 __unsetattr(int);
void	 __unset_color(WINDOW *win);
int	 __waddch(WINDOW *, __LDATA *);
int	 __wgetnstr(WINDOW *, char *, int);
void	 __winch_signal_handler(int);

/* Private #defines. */
#define	min(a,b)	((a) < (b) ? (a) : (b))
#define	max(a,b)	((a) > (b) ? (a ): (b))

/* Private externs. */
extern int		 __echoit;
extern int		 __endwin;
extern int		 __pfast;
extern int		 __rawmode;
extern int		 __noqch;
extern attr_t		 __mask_op, __mask_me, __mask_ue, __mask_se;
extern WINDOW		*__virtscr;
extern int		 __using_color;
extern attr_t		 __default_color;
