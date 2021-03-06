/* $NetBSD: spiflash.c,v 1.1 2006/10/07 07:21:13 gdamore Exp $ */

/*-
 * Copyright (c) 2006 Urbana-Champaign Independent Media Center.
 * Copyright (c) 2006 Garrett D'Amore.
 * All rights reserved.
 *
 * Portions of this code were written by Garrett D'Amore for the
 * Champaign-Urbana Community Wireless Network Project.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgements:
 *      This product includes software developed by the Urbana-Champaign
 *      Independent Media Center.
 *	This product includes software developed by Garrett D'Amore.
 * 4. Urbana-Champaign Independent Media Center's name and Garrett
 *    D'Amore's name may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE URBANA-CHAMPAIGN INDEPENDENT
 * MEDIA CENTER AND GARRETT D'AMORE ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE URBANA-CHAMPAIGN INDEPENDENT
 * MEDIA CENTER OR GARRETT D'AMORE BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: spiflash.c,v 1.1 2006/10/07 07:21:13 gdamore Exp $");

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/proc.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/kernel.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/disk.h>
#include <sys/disklabel.h>
#include <sys/buf.h>
#include <sys/bufq.h>
#include <sys/uio.h>
#include <sys/kthread.h>
#include <sys/malloc.h>
#include <sys/errno.h>

#include <dev/spi/spivar.h>
#include <dev/spi/spiflash.h>

/*
 * This is an MI block driver for SPI flash devices.  It could probably be
 * converted to some more generic framework, if someone wanted to create one
 * for NOR flashes.  Note that some flashes have the ability to handle
 * interrupts.
 */

struct spiflash_softc {
	struct device		sc_dev;
	struct disk		sc_dk;

	struct spiflash_hw_if	sc_hw;
	void			*sc_cookie;

	const char		*sc_name;
	struct spi_handle	*sc_handle;
	int			sc_device_size;
	int			sc_write_size;
	int			sc_erase_size;
	int			sc_read_size;
	int			sc_device_blks;

	struct bufq_state	*sc_bufq;
	struct proc		*sc_thread;
};

#define	sc_getname	sc_hw.sf_getname
#define	sc_gethandle	sc_hw.sf_gethandle
#define	sc_getsize	sc_hw.sf_getsize
#define	sc_getflags	sc_hw.sf_getflags
#define	sc_erase	sc_hw.sf_erase
#define	sc_write	sc_hw.sf_write
#define	sc_read		sc_hw.sf_read
#define	sc_getstatus	sc_hw.sf_getstatus
#define	sc_setstatus	sc_hw.sf_setstatus

struct spiflash_attach_args {
	const struct spiflash_hw_if	*hw;
	void				*cookie;
};

#define	STATIC
STATIC int spiflash_match(struct device *, struct cfdata *, void *);
STATIC void spiflash_attach(struct device *, struct device *, void *);
STATIC int spiflash_print(void *, const char *);
STATIC int spiflash_common_erase(spiflash_handle_t, size_t, size_t);
STATIC int spiflash_common_write(spiflash_handle_t, size_t, size_t,
    const uint8_t *);
STATIC int spiflash_common_read(spiflash_handle_t, size_t, size_t, uint8_t *);
STATIC void spiflash_process(spiflash_handle_t, struct buf *);
STATIC void spiflash_thread(void *);
STATIC void spiflash_thread_create(void *);

CFATTACH_DECL(spiflash, sizeof(struct spiflash_softc),
	      spiflash_match, spiflash_attach, NULL, NULL);

extern struct cfdriver spiflash_cd;

dev_type_open(spiflash_open);
dev_type_close(spiflash_close);
dev_type_read(spiflash_read);
dev_type_write(spiflash_write);
dev_type_ioctl(spiflash_ioctl);
dev_type_strategy(spiflash_strategy);

const struct bdevsw spiflash_bdevsw = {
	.d_open = spiflash_open,
	.d_close = spiflash_close,
	.d_strategy = spiflash_strategy,
	.d_ioctl = spiflash_ioctl,
	.d_dump = nodump,
	.d_psize = nosize,
	.d_type = D_DISK,
};

const struct cdevsw spiflash_cdevsw = {
	.d_open = spiflash_open,
	.d_close = spiflash_close,
	.d_read = spiflash_read,
	.d_write = spiflash_write,
	.d_ioctl = spiflash_ioctl,
	.d_stop = nostop,
	.d_tty = notty,
	.d_poll = nopoll,
	.d_mmap = nommap,
	.d_kqfilter = nokqfilter,
	.d_type = D_DISK,
};

static struct dkdriver spiflash_dkdriver = { spiflash_strategy, NULL };

spiflash_handle_t
spiflash_attach_mi(const struct spiflash_hw_if *hw, void *cookie,
    struct device *dev)
{
	struct spiflash_attach_args sfa;
	sfa.hw = hw;
	sfa.cookie = cookie;

	return (spiflash_handle_t)config_found(dev, &sfa, spiflash_print);
}

int
spiflash_print(void *aux, const char *pnp)
{
	if (pnp != NULL)
		printf("spiflash at %s\n", pnp);

	return UNCONF;
}

int
spiflash_match(struct device *parent, struct cfdata *cf, void *aux)
{

	return 1;
}

void
spiflash_attach(struct device *parent, struct device *self, void *aux)
{
	struct spiflash_softc *sc = device_private(self);
	struct spiflash_attach_args *sfa = aux;
	void *cookie = sfa->cookie;

	sc->sc_hw = *sfa->hw;
	sc->sc_cookie = cookie;
	sc->sc_name = sc->sc_getname(cookie);
	sc->sc_handle = sc->sc_gethandle(cookie);
	sc->sc_device_size = sc->sc_getsize(cookie, SPIFLASH_SIZE_DEVICE);
	sc->sc_erase_size = sc->sc_getsize(cookie, SPIFLASH_SIZE_ERASE);
	sc->sc_write_size = sc->sc_getsize(cookie, SPIFLASH_SIZE_WRITE);
	sc->sc_read_size = sc->sc_getsize(cookie, SPIFLASH_SIZE_READ);
	sc->sc_device_blks = sc->sc_device_size / DEV_BSIZE;

	if (sc->sc_read == NULL)
		sc->sc_read = spiflash_common_read;
	if (sc->sc_write == NULL)
		sc->sc_write = spiflash_common_write;
	if (sc->sc_erase == NULL)
		sc->sc_erase = spiflash_common_erase;

	aprint_naive(": SPI flash\n");
	aprint_normal(": %s SPI flash\n", sc->sc_name);
	/* XXX: note that this has to change for boot-sectored flash */
	aprint_normal("%s: %d KB, %d sectors of %d KB each\n",
	    sc->sc_dev.dv_xname, sc->sc_device_size / 1024,
	    sc->sc_device_size / sc->sc_erase_size,
	    sc->sc_erase_size / 1024);

	/* first-come first-served strategy works best for us */
	bufq_alloc(&sc->sc_bufq, "fcfs", BUFQ_SORT_RAWBLOCK);

	/* arrange to allocate the kthread */
	kthread_create(spiflash_thread_create, sc);

	sc->sc_dk.dk_driver = &spiflash_dkdriver;
	sc->sc_dk.dk_name = sc->sc_dev.dv_xname;

	disk_attach(&sc->sc_dk);
}

int
spiflash_open(dev_t dev, int flags, int mode, struct lwp *l)
{
	spiflash_handle_t sc;

	if ((sc = device_lookup(&spiflash_cd, DISKUNIT(dev))) == NULL)
		return ENXIO;

	/*
	 * XXX: We need to handle partitions here.  The problem is
	 * that it isn't entirely clear to me how to deal with this.
	 * There are devices that could be used "in the raw" with a
	 * NetBSD label, but then you get into devices that have other
	 * kinds of data on them -- some have VxWorks data, some have
	 * RedBoot data, and some have other contraints -- for example
	 * some devices might have a portion that is read-only,
	 * whereas others might have a portion that is read-write.
	 *
	 * For now we just permit access to the entire device.
	 */
	return 0;
}

int
spiflash_close(dev_t dev, int flags, int mode, struct lwp *l)
{
	spiflash_handle_t sc;

	if ((sc = device_lookup(&spiflash_cd, DISKUNIT(dev))) == NULL)
		return ENXIO;

	return 0;
}

int
spiflash_read(dev_t dev, struct uio *uio, int ioflag)
{

	return physio(spiflash_strategy, NULL, dev, B_READ, minphys, uio);
}

int
spiflash_write(dev_t dev, struct uio *uio, int ioflag)
{

	return physio(spiflash_strategy, NULL, dev, B_WRITE, minphys, uio);
}

int
spiflash_ioctl(dev_t dev, u_long cmd, caddr_t data, int flags, struct lwp *l)
{
	spiflash_handle_t sc;

	if ((sc = device_lookup(&spiflash_cd, DISKUNIT(dev))) == NULL)
		return ENXIO;

	return EINVAL;
}

void
spiflash_strategy(struct buf *bp)
{
	spiflash_handle_t sc;
	int	sz;
	int	s;

	sc = device_lookup(&spiflash_cd, DISKUNIT(bp->b_dev));
	if (sc == NULL) {
		bp->b_error = ENXIO;
		bp->b_flags |= B_ERROR;
		biodone(bp);
		return;
	}

	if ((bp->b_bcount % sc->sc_write_size) ||
	    (bp->b_blkno < 0)) {
		bp->b_error = EINVAL;
		bp->b_flags |= B_ERROR;
		biodone(bp);
		return;
	}

	/* no work? */
	if (bp->b_bcount == 0) {
		biodone(bp);
		return;
	}

	sz = bp->b_bcount / DEV_BSIZE;

	if ((bp->b_blkno + sz) > sc->sc_device_blks) {
		sz = sc->sc_device_blks - bp->b_blkno;
		/* exactly at end of media?  return EOF */
		if (sz == 0) {
			biodone(bp);
			return;
		}
		if (sz < 0) {
			/* past end of disk */
			bp->b_error = EINVAL;
			bp->b_flags |= B_ERROR;
			biodone(bp);
		}
		/* otherwise truncate it */
		bp->b_bcount = sz << DEV_BSHIFT;
	}

	bp->b_resid = bp->b_bcount;

	/* all ready, hand off to thread for async processing */
	s = splbio();
	BUFQ_PUT(sc->sc_bufq, bp);
	wakeup(&sc->sc_thread);
	splx(s);
}

void
spiflash_process(spiflash_handle_t sc, struct buf *bp)
{
	int	cnt;
	size_t	addr;
	uint8_t	*data;

	addr = bp->b_blkno * DEV_BSIZE;
	data = bp->b_data;

	while (bp->b_resid > 0) {
		cnt = max(sc->sc_write_size, DEV_BSIZE);
		if (bp->b_flags & B_READ) {
			bp->b_error = sc->sc_read(sc, addr, cnt, data);
		} else {
			bp->b_error = sc->sc_write(sc, addr, cnt, data);
		}
		if (bp->b_error) {
			bp->b_flags |= B_ERROR;
			biodone(bp);
			return;
		}
		bp->b_resid -= cnt;
		data += cnt;
		addr += cnt;
	}
	biodone(bp);
}

void
spiflash_thread(void *arg)
{
	spiflash_handle_t sc = arg;
	struct buf	*bp;
	int		s;

	s = splbio();
	for (;;) {
		if ((bp = BUFQ_GET(sc->sc_bufq)) == NULL) {
			tsleep(&sc->sc_thread, PRIBIO, "spiflash_thread", 0);
			continue;
		}

		spiflash_process(sc, bp);
	}
}

void
spiflash_thread_create(void *arg)
{
	spiflash_handle_t sc = arg;

	kthread_create1(spiflash_thread, arg, &sc->sc_thread,
	    "spiflash_thread");
}

/*
 * SPI flash common implementation.
 */

/*
 * Most devices take on the order of 1 second for each block that they
 * delete.
 */
int
spiflash_common_erase(spiflash_handle_t sc, size_t start, size_t size)
{
	int		rv;

	if ((start % sc->sc_erase_size) || (size % sc->sc_erase_size))
		return EINVAL;

	/* the second test is to test against wrap */ 
	if ((start > sc->sc_device_size) ||
	    ((start + size) > sc->sc_device_size))
		return EINVAL;

	/*
	 * XXX: check protection status?  Requires master table mapping
	 * sectors to status bits, and so forth.
	 */

	while (size) {
		if ((rv = spiflash_write_enable(sc)) != 0) {
			spiflash_write_disable(sc);
			return rv;
		}
		if ((rv = spiflash_cmd(sc, SPIFLASH_CMD_ERASE, 3, start, 0,
			 NULL, NULL)) != 0) {
			spiflash_write_disable(sc);
			return rv;
		}

		/*
		 * The devices I have all say typical for sector erase
		 * is ~1sec.  We check ten times that often.  (There
		 * is no way to interrupt on this.)
		 */
		if ((rv = spiflash_wait(sc, hz / 10)) != 0)
			return rv;

		start += sc->sc_erase_size;
		size -= sc->sc_erase_size;

		/* NB: according to the docs I have, the write enable
		 * is automatically cleared upon completion of an erase
		 * command, so there is no need to explicitly disable it.
		 */
	}

	return 0;
}

int
spiflash_common_write(spiflash_handle_t sc, size_t start, size_t size,
    const uint8_t *data)
{
	int		rv;

	if ((start % sc->sc_write_size) || (size % sc->sc_write_size))
		return EINVAL;

	/* the second test is to test against wrap */ 
	if ((start > sc->sc_device_size) ||
	    ((start + size) > sc->sc_device_size))
		return EINVAL;

	while (size) {
		int cnt;

		if ((rv = spiflash_write_enable(sc)) != 0) {
			spiflash_write_disable(sc);
			return rv;
		}

		cnt = min(size, sc->sc_write_size);
		if ((rv = spiflash_cmd(sc, SPIFLASH_CMD_PROGRAM, 3, start,
			 cnt, data, NULL)) != 0) {
			spiflash_write_disable(sc);
			return rv;
		}

		/*
		 * It seems that most devices can write bits fairly
		 * quickly.  For example, one part I have access to
		 * takes ~5msec to process the entire 256 byte page.
		 * Probably this should be modified to cope with
		 * device-specific timing, and maybe also take into
		 * account systems with higher values of HZ (which
		 * could benefit from sleeping.)
		 */
		if ((rv = spiflash_wait(sc, 0)) != 0)
			return rv;

		start += cnt;
		size -= cnt;
	}

	return 0;
}

int
spiflash_common_read(spiflash_handle_t sc, size_t start, size_t size,
    uint8_t *data)
{
	int		rv;
	int		align;

	align = sc->sc_write_size;
	if (sc->sc_read_size > 0)
		align = sc->sc_read_size;

	if ((start % align) || (size % align))
		return EINVAL;

	/* the second test is to test against wrap */ 
	if ((start > sc->sc_device_size) ||
	    ((start + size) > sc->sc_device_size))
		return EINVAL;

	while (size) {
		int cnt;

		if ((rv = spiflash_write_enable(sc)) != 0) {
			spiflash_write_disable(sc);
			return rv;
		}

		if (sc->sc_read_size > 0)
			cnt = min(size, sc->sc_read_size);
		else 
			cnt = size;

		if ((rv = spiflash_cmd(sc, SPIFLASH_CMD_READ, 3, start,
			 cnt, NULL, data)) != 0) {
			spiflash_write_disable(sc);
			return rv;
		}

		start += cnt;
		size -= cnt;
	}

	return 0;
}

/* read status register */
int
spiflash_read_status(spiflash_handle_t sc, uint8_t *sr)
{

	return spiflash_cmd(sc, SPIFLASH_CMD_RDSR, 0, 0, 1, NULL, sr);
}

int
spiflash_write_enable(spiflash_handle_t sc)
{

	return spiflash_cmd(sc, SPIFLASH_CMD_WREN, 0, 0, 0, NULL, NULL);
}

int
spiflash_write_disable(spiflash_handle_t sc)
{

	return spiflash_cmd(sc, SPIFLASH_CMD_WRDI, 0, 0, 0, NULL, NULL);
}

int
spiflash_cmd(spiflash_handle_t sc, uint8_t cmd,
    size_t addrlen, uint32_t addr,
    size_t cnt, const uint8_t *wdata, uint8_t *rdata)
{
	struct spi_transfer	trans;
	struct spi_chunk	chunk1, chunk2;
	char buf[4];
	int i;

	buf[0] = cmd;

	if (addrlen > 3)
		return EINVAL;

	for (i = addrlen; i > 0; i--) {
		buf[i] = (addr >> ((i - 1) * 8)) & 0xff;
	}
	spi_transfer_init(&trans);
	spi_chunk_init(&chunk1, addrlen + 1, buf, NULL);
	spi_transfer_add(&trans, &chunk1);
	if (cnt) {
		spi_chunk_init(&chunk2, cnt, wdata, rdata);
		spi_transfer_add(&trans, &chunk2);
	}

	spi_transfer(sc->sc_handle, &trans);
	spi_wait(&trans);

	if (trans.st_flags & SPI_F_ERROR)
		return trans.st_errno;
	return 0;
}

int
spiflash_wait(spiflash_handle_t sc, int tmo)
{
	int	rv;
	uint8_t	sr;

	for (;;) {
		if ((rv = spiflash_read_status(sc, &sr)) != 0)
			return rv;

		if ((sr & SPIFLASH_SR_BUSY) == 0)
			break;
		/*
		 * The devices I have all say typical for sector
		 * erase is ~1sec.  We check time times that often.
		 * (There is no way to interrupt on this.)
		 */
		if (tmo)
			tsleep(&sr, PWAIT, "spiflash_wait", tmo);
	}
	return 0;
}
