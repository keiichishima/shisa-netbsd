/*	$NetBSD: sunkbd.c,v 1.21 2005/02/21 22:43:07 heas Exp $	*/

/*
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This software was developed by the Computer Systems Engineering group
 * at Lawrence Berkeley Laboratory under DARPA contract BG 91-66 and
 * contributed to Berkeley.
 *
 * All advertising materials mentioning features or use of this software
 * must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Lawrence Berkeley Laboratory.
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
 *
 *	@(#)kbd.c	8.2 (Berkeley) 10/30/93
 */

/*
 * /dev/kbd lower layer for sun keyboard off a tty (line discipline).
 * This driver uses kbdsun middle layer to hook up to /dev/kbd.
 */

/*
 * Keyboard interface line discipline.
 *
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: sunkbd.c,v 1.21 2005/02/21 22:43:07 heas Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/device.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/proc.h>
#include <sys/signal.h>
#include <sys/signalvar.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/syslog.h>
#include <sys/fcntl.h>
#include <sys/tty.h>

#include <dev/cons.h>
#include <machine/vuid_event.h>
#include <machine/kbd.h>
#include <dev/sun/event_var.h>
#include <dev/sun/kbd_xlate.h>
#include <dev/sun/kbdvar.h>
#include <dev/sun/kbdsunvar.h>
#include <dev/sun/kbd_ms_ttyvar.h>

/****************************************************************
 * Interface to the lower layer (ttycc)
 ****************************************************************/

static int	sunkbd_match(struct device *, struct cfdata *, void *);
static void	sunkbd_attach(struct device *, struct device *, void *);
static void	sunkbd_write_data(struct kbd_sun_softc *, int);
static int	sunkbdiopen(struct device *, int mode);

#if NWSKBD > 0
void kbd_wskbd_attach(struct kbd_softc *k, int isconsole);
#endif

int	sunkbdinput(int, struct tty *);
int	sunkbdstart(struct tty *);

/* Default keyboard baud rate */
int	sunkbd_bps = KBD_DEFAULT_BPS;

CFATTACH_DECL(kbd_tty, sizeof(struct kbd_sun_softc),
    sunkbd_match, sunkbd_attach, NULL, NULL);

struct  linesw sunkbd_disc =
	{ "sunkbd", -1, ttylopen, ttylclose, ttyerrio, ttyerrio, ttynullioctl,
	  sunkbdinput, sunkbdstart, nullmodem, ttpoll };


/*
 * sunkbd_match: how is this tty channel configured?
 */
int
sunkbd_match(parent, cf, aux)
	struct device *parent;
	struct cfdata *cf;
	void   *aux;
{
	struct kbd_ms_tty_attach_args *args = aux;

	if (strcmp(args->kmta_name, "keyboard") == 0)
		return (1);

	return 0;
}

void
sunkbd_attach(parent, self, aux)
	struct device *parent, *self;
	void   *aux;
{
	struct kbd_sun_softc *k = (void *) self;
	struct kbd_ms_tty_attach_args *args = aux;
	struct tty *tp = args->kmta_tp;
	struct cons_channel *cc;

	/* Set up the proper line discipline. */
	ttyldisc_init();
	if (ttyldisc_add(&sunkbd_disc, -1) == -1)
		panic("sunkbd_attach: sunkbd_disc");
	tp->t_linesw = &sunkbd_disc;
	tp->t_oflag &= ~OPOST;
	tp->t_dev = args->kmta_dev;

	/* link the structures together. */
	k->k_priv = tp;
	tp->t_sc = (void *)k;

	/* provide our middle layer with a link to the lower layer (i.e. us) */
	k->k_deviopen = sunkbdiopen;
	k->k_deviclose = NULL;
	k->k_write_data = sunkbd_write_data;

	/* provide upper layer with a link to our middle layer */
	k->k_kbd.k_ops = &kbd_ops_sun;

	/* alloc console input channel */
	if ((cc = kbd_cc_alloc(&k->k_kbd)) == NULL)
		return;

	if (args->kmta_consdev) {
		char magic[4];

		/*
		 * Hookup ourselves as the console input channel
		 */
		args->kmta_baud = sunkbd_bps;
		args->kmta_cflag = CLOCAL|CS8;
		cons_attach_input(cc, args->kmta_consdev);

		/* Tell our parent what the console should be. */
		args->kmta_consdev = cn_tab;
		k->k_kbd.k_isconsole = 1;
		printf(" (console input)");

		/* Set magic to "L1-A" */
		magic[0] = KBD_L1;
		magic[1] = KBD_A;
		magic[2] = 0;
		cn_set_magic(magic);
	} else {
		extern void kd_attach_input(struct cons_channel *);

		kd_attach_input(cc);
	}


	printf("\n");

#if NWSKBD > 0
	kbd_wskbd_attach(&k->k_kbd, args->kmta_consdev != NULL);
#endif

	/* Do this before any calls to kbd_rint(). */
	kbd_xlate_init(&k->k_kbd.k_state);

	/* Magic sequence. */
	k->k_magic1 = KBD_L1;
	k->k_magic2 = KBD_A;
}

/*
 * Internal open routine.  This really should be inside com.c
 * But I'm putting it here until we have a generic internal open
 * mechanism.
 */
int
sunkbdiopen(dev, flags)
	struct device *dev;
	int flags;
{
	struct kbd_sun_softc *k = (void *) dev;
	struct tty *tp = (struct tty *)k->k_priv;
	struct proc *p = curproc ? curproc : &proc0;
	struct termios t;
	const struct cdevsw *cdev;
	int error;

	cdev = cdevsw_lookup(tp->t_dev);
	if (cdev == NULL)
		return (ENXIO);

	/* Open the lower device */
	if ((error = (*cdev->d_open)(tp->t_dev, O_NONBLOCK|flags,
				     0/* ignored? */, p)) != 0)
		return (error);

	/* Now configure it for the console. */
	tp->t_ospeed = 0;
	t.c_ispeed = sunkbd_bps;
	t.c_ospeed = sunkbd_bps;
	t.c_cflag =  CLOCAL|CS8;
	(*tp->t_param)(tp, &t);

	return (0);
}

/*
 * TTY interface to handle input.
 */
int
sunkbdinput(c, tp)
	int c;
	struct tty *tp;
{
	struct kbd_sun_softc *k = (void *) tp->t_sc;
	u_char *cc;
	int error;


	cc = tp->t_cc;

	/*
	 * Handle exceptional conditions (break, parity, framing).
	 */
	if ((error = ((c & TTY_ERRORMASK))) != 0) {
		/*
		 * After garbage, flush pending input, and
		 * send a reset to resync key translation.
		 */
		log(LOG_ERR, "%s: input error (0x%x)\n",
			k->k_kbd.k_dev.dv_xname, c);
		c &= TTY_CHARMASK;
		if (!k->k_txflags & K_TXBUSY) {
			ttyflush(tp, FREAD | FWRITE);
			goto send_reset;
		}
	}

	/*
	 * Check for input buffer overflow
	 */
	if (tp->t_rawq.c_cc + tp->t_canq.c_cc >= TTYHOG) {
		log(LOG_ERR, "%s: input overrun\n",
		    k->k_kbd.k_dev.dv_xname);
		goto send_reset;
	}

	/* Pass this up to the "middle" layer. */
	return(kbd_sun_input(k, c));

send_reset:
	/* Send a reset to resync translation. */
	kbd_sun_output(k, KBD_CMD_RESET);
	return (ttstart(tp));

}

int
sunkbdstart(tp)
	struct tty *tp;
{
	struct kbd_sun_softc *k = (void *) tp->t_sc;

	/*
	 * Transmit done.  Try to send more, or
	 * clear busy and wakeup drain waiters.
	 */
	k->k_txflags &= ~K_TXBUSY;
	kbd_sun_start_tx(k);
	ttstart(tp);
	return (0);
}
/*
 * used by kbd_sun_start_tx();
 */
void
sunkbd_write_data(k, c)
	struct kbd_sun_softc *k;
	int c;
{
	struct tty *tp = (struct tty *)k->k_priv;
	int	s;

	s = spltty();
	ttyoutput(c, tp);
	ttstart(tp);
	splx(s);
}
