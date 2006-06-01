/*	$NetBSD: dk.c,v 1.6 2005/02/27 00:26:59 perry Exp $	*/

/*-
 * Copyright (c) 2004 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jason R. Thorpe.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the NetBSD
 *	Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: dk.c,v 1.6 2005/02/27 00:26:59 perry Exp $");

#include "opt_dkwedge.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/errno.h>
#include <sys/pool.h>
#include <sys/ioctl.h>
#include <sys/disklabel.h>
#include <sys/disk.h>
#include <sys/fcntl.h>
#include <sys/buf.h>
#include <sys/bufq.h>
#include <sys/vnode.h>
#include <sys/stat.h>
#include <sys/conf.h>
#include <sys/callout.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/device.h>

#include <miscfs/specfs/specdev.h>

MALLOC_DEFINE(M_DKWEDGE, "dkwedge", "Disk wedge structures");

typedef enum {
	DKW_STATE_LARVAL	= 0,
	DKW_STATE_RUNNING	= 1,
	DKW_STATE_DYING		= 2,
	DKW_STATE_DEAD		= 666
} dkwedge_state_t;

struct dkwedge_softc {
	struct device	*sc_dev;	/* pointer to our pseudo-device */
	struct cfdata	sc_cfdata;	/* our cfdata structure */
	uint8_t		sc_wname[128];	/* wedge name (Unicode, UTF-8) */

	dkwedge_state_t sc_state;	/* state this wedge is in */

	struct disk	*sc_parent;	/* parent disk */
	daddr_t		sc_offset;	/* LBA offset of wedge in parent */
	uint64_t	sc_size;	/* size of wedge in blocks */
	char		sc_ptype[32];	/* partition type */
	dev_t		sc_pdev;	/* cached parent's dev_t */
					/* link on parent's wedge list */
	LIST_ENTRY(dkwedge_softc) sc_plink;

	struct disk	sc_dk;		/* our own disk structure */
	struct bufq_state sc_bufq;	/* buffer queue */
	struct callout	sc_restart_ch;	/* callout to restart I/O */

	u_int		sc_iopend;	/* I/Os pending */
	int		sc_flags;	/* flags (splbio) */
};

#define	DK_F_WAIT_DRAIN		0x0001	/* waiting for I/O to drain */

static void	dkstart(struct dkwedge_softc *);
static void	dkiodone(struct buf *);
static void	dkrestart(void *);

static dev_type_open(dkopen);
static dev_type_close(dkclose);
static dev_type_read(dkread);
static dev_type_write(dkwrite);
static dev_type_ioctl(dkioctl);
static dev_type_strategy(dkstrategy);
static dev_type_dump(dkdump);
static dev_type_size(dksize);

const struct bdevsw dk_bdevsw = {
	dkopen, dkclose, dkstrategy, dkioctl, dkdump, dksize, D_DISK
};

const struct cdevsw dk_cdevsw = {
	dkopen, dkclose, dkread, dkwrite, dkioctl,
	    nostop, notty, nopoll, nommap, nokqfilter, D_DISK
};

static struct dkwedge_softc **dkwedges;
static u_int ndkwedges;
static struct lock dkwedges_lock = LOCK_INITIALIZER(PRIBIO, "dkwgs", 0, 0);

static LIST_HEAD(, dkwedge_discovery_method) dkwedge_discovery_methods;
static int dkwedge_discovery_methods_initialized;
static struct lock dkwedge_discovery_methods_lock =
    LOCK_INITIALIZER(PRIBIO, "dkddm", 0, 0);

/*
 * dkwedge_match:
 *
 *	Autoconfiguration match function for pseudo-device glue.
 */
static int
dkwedge_match(struct device *parent, struct cfdata *match, void *aux)
{

	/* Pseudo-device; always present. */
	return (1);
}

/*
 * dkwedge_attach:
 *
 *	Autoconfiguration attach function for pseudo-device glue.
 */
static void
dkwedge_attach(struct device *parent, struct device *self, void *aux)
{

	/* Nothing to do. */
}

/*
 * dkwedge_detach:
 *
 *	Autoconfiguration detach function for pseudo-device glue.
 */
static int
dkwedge_detach(struct device *self, int flags)
{

	/* Always succeeds. */
	return (0);
}

CFDRIVER_DECL(dk, DV_DISK, NULL);
CFATTACH_DECL(dk, sizeof(struct device),
	      dkwedge_match, dkwedge_attach, dkwedge_detach, NULL);

static int dkwedge_cfglue_initialized;
static struct simplelock dkwedge_cfglue_initialized_slock =
    SIMPLELOCK_INITIALIZER;

static void
dkwedge_cfglue_init(void)
{

	simple_lock(&dkwedge_cfglue_initialized_slock);
	if (dkwedge_cfglue_initialized == 0) {
		if (config_cfdriver_attach(&dk_cd) != 0)
			panic("dkwedge: unable to attach cfdriver");
		if (config_cfattach_attach(dk_cd.cd_name, &dk_ca) != 0)
			panic("dkwedge: unable to attach cfattach");

		dkwedge_cfglue_initialized = 1;
	}
	simple_unlock(&dkwedge_cfglue_initialized_slock);
}

/*
 * dkwedge_wait_drain:
 *
 *	Wait for I/O on the wedge to drain.
 *	NOTE: Must be called at splbio()!
 */
static void
dkwedge_wait_drain(struct dkwedge_softc *sc)
{

	while (sc->sc_iopend != 0) {
		sc->sc_flags |= DK_F_WAIT_DRAIN;
		(void) tsleep(&sc->sc_iopend, PRIBIO, "dkdrn", 0);
	}
}

/*
 * dkwedge_compute_pdev:
 *
 *	Compute the parent disk's dev_t.
 */
static int
dkwedge_compute_pdev(const char *pname, dev_t *pdevp)
{
	const char *name, *cp;
	int punit, pmaj;
	char devname[16];

	name = pname;
	if ((pmaj = devsw_name2blk(name, devname, sizeof(devname))) == -1)
		return (ENODEV);

	name += strlen(devname);
	for (cp = name, punit = 0; *cp >= '0' && *cp <= '9'; cp++)
		punit = (punit * 10) + (*cp - '0');
	if (cp == name) {
		/* Invalid parent disk name. */
		return (ENODEV);
	}

	*pdevp = MAKEDISKDEV(pmaj, punit, RAW_PART);

	return (0);
}

/*
 * dkwedge_array_expand:
 *
 *	Expand the dkwedges array.
 */
static void
dkwedge_array_expand(void)
{
	int newcnt = ndkwedges + 16;
	struct dkwedge_softc **newarray, **oldarray;

	newarray = malloc(newcnt * sizeof(*newarray), M_DKWEDGE,
	    M_WAITOK|M_ZERO);
	if ((oldarray = dkwedges) != NULL)
		memcpy(newarray, dkwedges, ndkwedges * sizeof(*newarray));
	dkwedges = newarray;
	ndkwedges = newcnt;
	if (oldarray != NULL)
		free(oldarray, M_DKWEDGE);
}

/*
 * dkwedge_add:		[exported function]
 *
 *	Add a disk wedge based on the provided information.
 *
 *	The incoming dkw_devname[] is ignored, instead being
 *	filled in and returned to the caller.
 */
int
dkwedge_add(struct dkwedge_info *dkw)
{
	struct dkwedge_softc *sc, *lsc;
	struct disk *pdk;
	u_int unit;
	int error;
	dev_t pdev;

	if (dkwedge_cfglue_initialized == 0)
		dkwedge_cfglue_init();

	dkw->dkw_parent[sizeof(dkw->dkw_parent) - 1] = '\0';
	pdk = disk_find(dkw->dkw_parent);
	if (pdk == NULL)
		return (ENODEV);

	error = dkwedge_compute_pdev(pdk->dk_name, &pdev);
	if (error)
		return (error);

	if (dkw->dkw_offset < 0)
		return (EINVAL);

	sc = malloc(sizeof(*sc), M_DKWEDGE, M_WAITOK|M_ZERO);
	sc->sc_state = DKW_STATE_LARVAL;
	sc->sc_parent = pdk;
	sc->sc_pdev = pdev;
	sc->sc_offset = dkw->dkw_offset;
	sc->sc_size = dkw->dkw_size;

	memcpy(sc->sc_wname, dkw->dkw_wname, sizeof(sc->sc_wname));
	sc->sc_wname[sizeof(sc->sc_wname) - 1] = '\0';

	memcpy(sc->sc_ptype, dkw->dkw_ptype, sizeof(sc->sc_ptype));
	sc->sc_ptype[sizeof(sc->sc_ptype) - 1] = '\0';

	bufq_alloc(&sc->sc_bufq, BUFQ_FCFS);

	callout_init(&sc->sc_restart_ch);
	callout_setfunc(&sc->sc_restart_ch, dkrestart, sc);

	/*
	 * Wedge will be added; increment the wedge count for the parent.
	 * Only allow this to happend if RAW_PART is the only thing open.
	 */
	(void) lockmgr(&pdk->dk_openlock, LK_EXCLUSIVE, NULL);
	if (pdk->dk_openmask & ~(1 << RAW_PART))
		error = EBUSY;
	else {
		/* Check for wedge overlap. */
		LIST_FOREACH(lsc, &pdk->dk_wedges, sc_plink) {
			daddr_t lastblk = sc->sc_offset + sc->sc_size - 1;
			daddr_t llastblk = lsc->sc_offset + lsc->sc_size - 1;

			if (sc->sc_offset >= lsc->sc_offset &&
			    sc->sc_offset <= llastblk) {
				/* Overlaps the tail of the exsiting wedge. */
				break;
			}
			if (lastblk >= lsc->sc_offset &&
			    lastblk <= llastblk) {
				/* Overlaps the head of the existing wedge. */
			    	break;
			}
		}
		if (lsc != NULL)
			error = EINVAL;
		else {
			pdk->dk_nwedges++;
			LIST_INSERT_HEAD(&pdk->dk_wedges, sc, sc_plink);
		}
	}
	(void) lockmgr(&pdk->dk_openlock, LK_RELEASE, NULL);
	if (error) {
		bufq_free(&sc->sc_bufq);
		free(sc, M_DKWEDGE);
		return (error);
	}

	/* Fill in our cfdata for the pseudo-device glue. */
	sc->sc_cfdata.cf_name = dk_cd.cd_name;
	sc->sc_cfdata.cf_atname = dk_ca.ca_name;
	/* sc->sc_cfdata.cf_unit set below */
	sc->sc_cfdata.cf_fstate = FSTATE_NOTFOUND;

	/* Insert the larval wedge into the array. */
	(void) lockmgr(&dkwedges_lock, LK_EXCLUSIVE, NULL);
	for (error = 0;;) {
		struct dkwedge_softc **scpp;

		/*
		 * Check for a duplicate wname while searching for
		 * a slot.
		 */
		for (scpp = NULL, unit = 0; unit < ndkwedges; unit++) {
			if (dkwedges[unit] == NULL) {
				if (scpp == NULL) {
					scpp = &dkwedges[unit];
					sc->sc_cfdata.cf_unit = unit;
				}
			} else {
				/* XXX Unicode. */
				if (strcmp(dkwedges[unit]->sc_wname,
					   sc->sc_wname) == 0) {
					error = EEXIST;
					break;
				}
			}
		}
		if (error)
			break;
		KASSERT(unit == ndkwedges);
		if (scpp == NULL)
			dkwedge_array_expand();
		else {
			KASSERT(scpp == &dkwedges[sc->sc_cfdata.cf_unit]);
			*scpp = sc;
			break;
		}
	}
	(void) lockmgr(&dkwedges_lock, LK_RELEASE, NULL);
	if (error) {
		(void) lockmgr(&pdk->dk_openlock, LK_EXCLUSIVE, NULL);
		pdk->dk_nwedges--;
		LIST_REMOVE(sc, sc_plink);
		(void) lockmgr(&pdk->dk_openlock, LK_RELEASE, NULL);

		bufq_free(&sc->sc_bufq);
		free(sc, M_DKWEDGE);
		return (error);
	}

	/*
	 * Now that we know the unit #, attach a pseudo-device for
	 * this wedge instance.  This will provide us with the
	 * "struct device" necessary for glue to other parts of the
	 * system.
	 *
	 * This should never fail, unless we're almost totally out of
	 * memory.
	 */
	if ((sc->sc_dev = config_attach_pseudo(&sc->sc_cfdata)) == NULL) {
		aprint_error("%s%u: unable to attach pseudo-device\n",
		    sc->sc_cfdata.cf_name, sc->sc_cfdata.cf_unit);

		(void) lockmgr(&dkwedges_lock, LK_EXCLUSIVE, NULL);
		dkwedges[sc->sc_cfdata.cf_unit] = NULL;
		(void) lockmgr(&dkwedges_lock, LK_RELEASE, NULL);

		(void) lockmgr(&pdk->dk_openlock, LK_EXCLUSIVE, NULL);
		pdk->dk_nwedges--;
		LIST_REMOVE(sc, sc_plink);
		(void) lockmgr(&pdk->dk_openlock, LK_RELEASE, NULL);

		bufq_free(&sc->sc_bufq);
		free(sc, M_DKWEDGE);
		return (ENOMEM);
	}
	sc->sc_dk.dk_name = sc->sc_dev->dv_xname;

	/* Return the devname to the caller. */
	strcpy(dkw->dkw_devname, sc->sc_dev->dv_xname);

	/*
	 * XXX Really ought to make the disk_attach() and the changing
	 * of state to RUNNING atomic.
	 */

	disk_attach(&sc->sc_dk);

	/* Disk wedge is ready for use! */
	sc->sc_state = DKW_STATE_RUNNING;

	/* Announce our arrival. */
	aprint_normal("%s at %s: %s\n", sc->sc_dev->dv_xname, pdk->dk_name,
	    sc->sc_wname);	/* XXX Unicode */
	aprint_normal("%s: %"PRIu64" blocks at %"PRId64", type: %s\n",
	    sc->sc_dev->dv_xname, sc->sc_size, sc->sc_offset, sc->sc_ptype);

	return (0);
}

/*
 * dkwedge_del:		[exported function]
 *
 *	Delete a disk wedge based on the provided information.
 *	NOTE: We look up the wedge based on the wedge devname,
 *	not wname.
 */
int
dkwedge_del(struct dkwedge_info *dkw)
{
	struct dkwedge_softc *sc = NULL;
	u_int unit;
	int bmaj, cmaj, i, mn, s;

	/* Find our softc. */
	dkw->dkw_devname[sizeof(dkw->dkw_devname) - 1] = '\0';
	(void) lockmgr(&dkwedges_lock, LK_EXCLUSIVE, NULL);
	for (unit = 0; unit < ndkwedges; unit++) {
		if ((sc = dkwedges[unit]) != NULL &&
		    strcmp(sc->sc_dev->dv_xname, dkw->dkw_devname) == 0 &&
		    strcmp(sc->sc_parent->dk_name, dkw->dkw_parent) == 0) {
			/* Mark the wedge as dying. */
			sc->sc_state = DKW_STATE_DYING;
			break;
		}
	}
	(void) lockmgr(&dkwedges_lock, LK_RELEASE, NULL);
	if (unit == ndkwedges)
		return (ESRCH);

	KASSERT(sc != NULL);

	/* Locate the wedge major numbers. */
	bmaj = bdevsw_lookup_major(&dk_bdevsw);
	cmaj = cdevsw_lookup_major(&dk_cdevsw);

	/* Kill any pending restart. */
	callout_stop(&sc->sc_restart_ch);

	/*
	 * dkstart() will kill any queued buffers now that the
	 * state of the wedge is not RUNNING.  Once we've done
	 * that, wait for any other pending I/O to complete.
	 */
	s = splbio();
	dkstart(sc);
	dkwedge_wait_drain(sc);
	splx(s);

	/* Nuke the vnodes for any open instances. */
	for (i = 0; i < MAXPARTITIONS; i++) {
		mn = DISKMINOR(unit, i);
		vdevgone(bmaj, mn, mn, VBLK);
		vdevgone(cmaj, mn, mn, VCHR);
	}

	/* Clean up the parent. */
	(void) lockmgr(&sc->sc_dk.dk_openlock, LK_EXCLUSIVE, NULL);
	(void) lockmgr(&sc->sc_parent->dk_rawlock, LK_EXCLUSIVE, NULL);
	if (sc->sc_dk.dk_openmask) {
		if (sc->sc_parent->dk_rawopens-- == 1) {
			KASSERT(sc->sc_parent->dk_rawvp != NULL);
			(void) vn_close(sc->sc_parent->dk_rawvp, FREAD | FWRITE,
					NOCRED, curproc);
			sc->sc_parent->dk_rawvp = NULL;
		}
		sc->sc_dk.dk_openmask = 0;
	}
	(void) lockmgr(&sc->sc_parent->dk_rawlock, LK_RELEASE, NULL);
	(void) lockmgr(&sc->sc_dk.dk_openlock, LK_RELEASE, NULL);

	/* Announce our departure. */
	aprint_normal("%s at %s (%s) deleted\n", sc->sc_dev->dv_xname,
	    sc->sc_parent->dk_name,
	    sc->sc_wname);	/* XXX Unicode */

	/* Delete our pseudo-device. */
	(void) config_detach(sc->sc_dev, DETACH_FORCE | DETACH_QUIET);

	(void) lockmgr(&sc->sc_parent->dk_openlock, LK_EXCLUSIVE, NULL);
	sc->sc_parent->dk_nwedges--;
	LIST_REMOVE(sc, sc_plink);
	(void) lockmgr(&sc->sc_parent->dk_openlock, LK_RELEASE, NULL);

	/* Delete our buffer queue. */
	bufq_free(&sc->sc_bufq);

	/* Detach from the disk list. */
	disk_detach(&sc->sc_dk);

	/* Poof. */
	(void) lockmgr(&dkwedges_lock, LK_EXCLUSIVE, NULL);
	dkwedges[unit] = NULL;
	sc->sc_state = DKW_STATE_DEAD;
	(void) lockmgr(&dkwedges_lock, LK_RELEASE, NULL);

	free(sc, M_DKWEDGE);

	return (0);
}

/*
 * dkwedge_delall:	[exported function]
 *
 *	Delete all of the wedges on the specified disk.  Used when
 *	a disk is being detached.
 */
void
dkwedge_delall(struct disk *pdk)
{
	struct dkwedge_info dkw;
	struct dkwedge_softc *sc;

	for (;;) {
		(void) lockmgr(&pdk->dk_openlock, LK_EXCLUSIVE, NULL);
		if ((sc = LIST_FIRST(&pdk->dk_wedges)) == NULL) {
			KASSERT(pdk->dk_nwedges == 0);
			(void) lockmgr(&pdk->dk_openlock, LK_RELEASE, NULL);
			return;
		}
		strcpy(dkw.dkw_parent, pdk->dk_name);
		strcpy(dkw.dkw_devname, sc->sc_dev->dv_xname);
		(void) lockmgr(&pdk->dk_openlock, LK_RELEASE, NULL);
		(void) dkwedge_del(&dkw);
	}
}

/*
 * dkwedge_list:	[exported function]
 *
 *	List all of the wedges on a particular disk.
 *	If p == NULL, the buffer is in kernel space.  Otherwise, it is
 *	in user space of the specified process.
 */
int
dkwedge_list(struct disk *pdk, struct dkwedge_list *dkwl, struct proc *p)
{
	struct uio uio;
	struct iovec iov;
	struct dkwedge_softc *sc;
	struct dkwedge_info dkw;
	int error = 0;

	iov.iov_base = dkwl->dkwl_buf;
	iov.iov_len = dkwl->dkwl_bufsize;

	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_offset = 0;
	uio.uio_resid = dkwl->dkwl_bufsize;
	uio.uio_segflg = p != NULL ? UIO_USERSPACE : UIO_SYSSPACE;
	uio.uio_rw = UIO_READ;
	uio.uio_procp = p;

	dkwl->dkwl_ncopied = 0;

	(void) lockmgr(&pdk->dk_openlock, LK_EXCLUSIVE, NULL);
	LIST_FOREACH(sc, &pdk->dk_wedges, sc_plink) {
		if (uio.uio_resid < sizeof(dkw))
			break;

		if (sc->sc_state != DKW_STATE_RUNNING)
			continue;

		strcpy(dkw.dkw_devname, sc->sc_dev->dv_xname);
		memcpy(dkw.dkw_wname, sc->sc_wname, sizeof(dkw.dkw_wname));
		dkw.dkw_wname[sizeof(dkw.dkw_wname) - 1] = '\0';
		strcpy(dkw.dkw_parent, sc->sc_parent->dk_name);
		dkw.dkw_offset = sc->sc_offset;
		dkw.dkw_size = sc->sc_size;
		strcpy(dkw.dkw_ptype, sc->sc_ptype);

		error = uiomove(&dkw, sizeof(dkw), &uio);
		if (error)
			break;
		dkwl->dkwl_ncopied++;
	}
	dkwl->dkwl_nwedges = pdk->dk_nwedges;
	(void) lockmgr(&pdk->dk_openlock, LK_RELEASE, NULL);

	return (error);
}

/*
 * dkwedge_set_bootwedge
 *
 *	Set the booted_wedge global based on the specified parent name
 *	and offset/length.
 */
void
dkwedge_set_bootwedge(struct device *parent, daddr_t startblk, uint64_t nblks)
{
	struct dkwedge_softc *sc;
	int i;

	(void) lockmgr(&dkwedges_lock, LK_EXCLUSIVE, NULL);
	for (i = 0; i < ndkwedges; i++) {
		if ((sc = dkwedges[i]) == NULL)
			continue;
		if (strcmp(sc->sc_parent->dk_name, parent->dv_xname) == 0 &&
		    sc->sc_offset == startblk &&
		    sc->sc_size == nblks) {
			if (booted_wedge) {
				printf("WARNING: double match for boot wedge "
				    "(%s, %s)\n",
				    booted_wedge->dv_xname,
				    sc->sc_dev->dv_xname);
				continue;
			}
			booted_device = parent;
			booted_wedge = sc->sc_dev;
			booted_partition = 0;
		}
	}
	/*
	 * XXX What if we don't find one?  Should we create a special
	 * XXX root wedge?
	 */
	(void) lockmgr(&dkwedges_lock, LK_RELEASE, NULL);
}

/*
 * We need a dummy objet to stuff into the dkwedge discovery method link
 * set to ensure that there is always at least one object in the set.
 */
static struct dkwedge_discovery_method dummy_discovery_method;
__link_set_add_bss(dkwedge_methods, dummy_discovery_method);

/*
 * dkwedge_discover_init:
 *
 *	Initialize the disk wedge discovery method list.
 */
static void
dkwedge_discover_init(void)
{
	__link_set_decl(dkwedge_methods, struct dkwedge_discovery_method);
	struct dkwedge_discovery_method * const *ddmp;
	struct dkwedge_discovery_method *lddm, *ddm;

	(void) lockmgr(&dkwedge_discovery_methods_lock, LK_EXCLUSIVE, NULL);

	if (dkwedge_discovery_methods_initialized) {
		(void) lockmgr(&dkwedge_discovery_methods_lock, LK_RELEASE,
			       NULL);
		return;
	}

	LIST_INIT(&dkwedge_discovery_methods);

	__link_set_foreach(ddmp, dkwedge_methods) {
		ddm = *ddmp;
		if (ddm == &dummy_discovery_method)
			continue;
		if (LIST_EMPTY(&dkwedge_discovery_methods)) {
			LIST_INSERT_HEAD(&dkwedge_discovery_methods,
					 ddm, ddm_list);
			continue;
		}
		LIST_FOREACH(lddm, &dkwedge_discovery_methods, ddm_list) {
			if (ddm->ddm_priority == lddm->ddm_priority) {
				aprint_error("dk-method-%s: method \"%s\" "
				    "already exists at priority %d\n",
				    ddm->ddm_name, lddm->ddm_name,
				    lddm->ddm_priority);
				/* Not inserted. */
				break;
			}
			if (ddm->ddm_priority < lddm->ddm_priority) {
				/* Higher priority; insert before. */
				LIST_INSERT_BEFORE(lddm, ddm, ddm_list);
				break;
			}
			if (LIST_NEXT(lddm, ddm_list) == NULL) {
				/* Last one; insert after. */
				KASSERT(lddm->ddm_priority < ddm->ddm_priority);
				LIST_INSERT_AFTER(lddm, ddm, ddm_list);
				break;
			}
		}
	}

	dkwedge_discovery_methods_initialized = 1;

	(void) lockmgr(&dkwedge_discovery_methods_lock, LK_RELEASE, NULL);
}

#ifdef DKWEDGE_AUTODISCOVER
int	dkwedge_autodiscover = 1;
#else
int	dkwedge_autodiscover = 0;
#endif

/*
 * dkwedge_discover:	[exported function]
 *
 *	Discover the wedges on a newly attached disk.
 */
void
dkwedge_discover(struct disk *pdk)
{
	struct dkwedge_discovery_method *ddm;
	struct vnode *vp;
	int error;
	dev_t pdev;

	/*
	 * Require people playing with wedges to enable this explicitly.
	 */
	if (dkwedge_autodiscover == 0)
		return;

	if (dkwedge_discovery_methods_initialized == 0)
		dkwedge_discover_init();

	(void) lockmgr(&dkwedge_discovery_methods_lock, LK_SHARED, NULL);

	error = dkwedge_compute_pdev(pdk->dk_name, &pdev);
	if (error) {
		aprint_error("%s: unable to compute pdev, error = %d\n",
		    pdk->dk_name, error);
		goto out;
	}

	error = bdevvp(pdev, &vp);
	if (error) {
		aprint_error("%s: unable to find vnode for pdev, error = %d\n",
		    pdk->dk_name, error);
		goto out;
	}

	error = vn_lock(vp, LK_EXCLUSIVE | LK_RETRY);
	if (error) {
		aprint_error("%s: unable to lock vnode for pdev, error = %d\n",
		    pdk->dk_name, error);
		vrele(vp);
		goto out;
	}

	error = VOP_OPEN(vp, FREAD | FWRITE, NOCRED, 0);
	if (error) {
		aprint_error("%s: unable to open device, error = %d\n",
		    pdk->dk_name, error);
		vput(vp);
		goto out;
	}
	/* VOP_OPEN() doesn't do this for us. */
	vp->v_writecount++;
	VOP_UNLOCK(vp, 0);

	/*
	 * For each supported partition map type, look to see if
	 * this map type exists.  If so, parse it and add the
	 * corresponding wedges.
	 */
	LIST_FOREACH(ddm, &dkwedge_discovery_methods, ddm_list) {
		error = (*ddm->ddm_discover)(pdk, vp);
		if (error == 0) {
			/* Successfully created wedges; we're done. */
			break;
		}
	}

	error = vn_close(vp, FREAD | FWRITE, NOCRED, curproc);
	if (error) {
		aprint_error("%s: unable to close device, error = %d\n",
		    pdk->dk_name, error);
		/* We'll just assume the vnode has been cleaned up. */
	}
 out:
	(void) lockmgr(&dkwedge_discovery_methods_lock, LK_RELEASE, NULL);
}

/*
 * dkwedge_read:
 *
 *	Read the some data from the specified disk, used for
 *	partition discovery.
 */
int
dkwedge_read(struct disk *pdk, struct vnode *vp, daddr_t blkno, void *buf,
    size_t len)
{
	struct buf b;

	BUF_INIT(&b);

	b.b_vp = vp;
	b.b_dev = vp->v_rdev;
	b.b_blkno = blkno;
	b.b_bcount = b.b_resid = len;
	b.b_flags = B_READ;
	b.b_proc = curproc;
	b.b_data = buf;

	VOP_STRATEGY(vp, &b);
	return (biowait(&b));
}

/*
 * dkwedge_lookup:
 *
 *	Look up a dkwedge_softc based on the provided dev_t.
 */
static struct dkwedge_softc *
dkwedge_lookup(dev_t dev)
{
	int unit = minor(dev);

	if (unit >= ndkwedges)
		return (NULL);

	KASSERT(dkwedges != NULL);

	return (dkwedges[unit]);
}

/*
 * dkopen:		[devsw entry point]
 *
 *	Open a wedge.
 */
static int
dkopen(dev_t dev, int flags, int fmt, struct proc *p)
{
	struct dkwedge_softc *sc = dkwedge_lookup(dev);
	struct vnode *vp;
	int error;

	if (sc == NULL)
		return (ENODEV);

	if (sc->sc_state != DKW_STATE_RUNNING)
		return (ENXIO);

	/*
	 * We go through a complicated little dance to only open the parent
	 * vnode once per wedge, no matter how many times the wedge is
	 * opened.  The reason?  We see one dkopen() per open call, but
	 * only dkclose() on the last close.
	 */
	(void) lockmgr(&sc->sc_dk.dk_openlock, LK_EXCLUSIVE, NULL);
	(void) lockmgr(&sc->sc_parent->dk_rawlock, LK_EXCLUSIVE, NULL);
	if (sc->sc_dk.dk_openmask == 0) {
		if (sc->sc_parent->dk_rawopens++ == 0) {
			KASSERT(sc->sc_parent->dk_rawvp == NULL);
			error = bdevvp(sc->sc_pdev, &vp);
			if (error)
				goto popen_fail;
			error = vn_lock(vp, LK_EXCLUSIVE | LK_RETRY);
			if (error) {
				vrele(vp);
				goto popen_fail;
			}
			error = VOP_OPEN(vp, FREAD | FWRITE, NOCRED, 0);
			if (error) {
				vput(vp);
				goto popen_fail;
			}
			/* VOP_OPEN() doesn't do this for us. */
			vp->v_writecount++;
			VOP_UNLOCK(vp, 0);
			sc->sc_parent->dk_rawvp = vp;
		}
		if (fmt == S_IFCHR)
			sc->sc_dk.dk_copenmask |= 1;
		else
			sc->sc_dk.dk_bopenmask |= 1;
		sc->sc_dk.dk_openmask =
		    sc->sc_dk.dk_copenmask | sc->sc_dk.dk_bopenmask;
	}
	(void) lockmgr(&sc->sc_parent->dk_rawlock, LK_RELEASE, NULL);
	(void) lockmgr(&sc->sc_dk.dk_openlock, LK_RELEASE, NULL);

	return (0);

 popen_fail:
	(void) lockmgr(&sc->sc_parent->dk_rawlock, LK_RELEASE, NULL);
	return (error);
}

/*
 * dkclose:		[devsw entry point]
 *
 *	Close a wedge.
 */
static int
dkclose(dev_t dev, int flags, int fmt, struct proc *p)
{
	struct dkwedge_softc *sc = dkwedge_lookup(dev);
	int error = 0;

	KASSERT(sc->sc_dk.dk_openmask != 0);

	(void) lockmgr(&sc->sc_dk.dk_openlock, LK_EXCLUSIVE, NULL);
	(void) lockmgr(&sc->sc_parent->dk_rawlock, LK_EXCLUSIVE, NULL);

	if (fmt == S_IFCHR)
		sc->sc_dk.dk_copenmask &= ~1;
	else
		sc->sc_dk.dk_bopenmask &= ~1;
	sc->sc_dk.dk_openmask =
	    sc->sc_dk.dk_copenmask | sc->sc_dk.dk_bopenmask;

	if (sc->sc_dk.dk_openmask == 0) {
		if (sc->sc_parent->dk_rawopens-- == 1) {
			KASSERT(sc->sc_parent->dk_rawvp != NULL);
			error = vn_close(sc->sc_parent->dk_rawvp,
					 FREAD | FWRITE, NOCRED, p);
			sc->sc_parent->dk_rawvp = NULL;
		}
	}

	(void) lockmgr(&sc->sc_parent->dk_rawlock, LK_RELEASE, NULL);
	(void) lockmgr(&sc->sc_dk.dk_openlock, LK_RELEASE, NULL);

	return (error);
}

/*
 * dkstragegy:		[devsw entry point]
 *
 *	Perform I/O based on the wedge I/O strategy.
 */
static void
dkstrategy(struct buf *bp)
{
	struct dkwedge_softc *sc = dkwedge_lookup(bp->b_dev);
	int s;

	if (sc->sc_state != DKW_STATE_RUNNING) {
		bp->b_error = ENXIO;
		bp->b_flags |= B_ERROR;
		goto done;
	}

	/* If it's an empty transfer, wake up the top half now. */
	if (bp->b_bcount == 0)
		goto done;

	/* Make sure it's in-range. */
	if (bounds_check_with_mediasize(bp, DEV_BSIZE, sc->sc_size) <= 0)
		goto done;

	/* Translate it to the parent's raw LBA. */
	bp->b_rawblkno = bp->b_blkno + sc->sc_offset;

	/* Place it in the queue and start I/O on the unit. */
	s = splbio();
	sc->sc_iopend++;
	BUFQ_PUT(&sc->sc_bufq, bp);
	dkstart(sc);
	splx(s);
	return;

 done:
	bp->b_resid = bp->b_bcount;
	biodone(bp);
}

/*
 * dkstart:
 *
 *	Start I/O that has been enqueued on the wedge.
 *	NOTE: Must be called at splbio()!
 */
static void
dkstart(struct dkwedge_softc *sc)
{
	struct buf *bp, *nbp;

	/* Do as much work as has been enqueued. */
	while ((bp = BUFQ_PEEK(&sc->sc_bufq)) != NULL) {
		if (sc->sc_state != DKW_STATE_RUNNING) {
			(void) BUFQ_GET(&sc->sc_bufq);
			if (sc->sc_iopend-- == 1 &&
			    (sc->sc_flags & DK_F_WAIT_DRAIN) != 0) {
				sc->sc_flags &= ~DK_F_WAIT_DRAIN;
				wakeup(&sc->sc_iopend);
			}
			bp->b_error = ENXIO;
			bp->b_flags |= B_ERROR;
			bp->b_resid = bp->b_bcount;
			biodone(bp);
		}

		/* Instrumentation. */
		disk_busy(&sc->sc_dk);

		nbp = pool_get(&bufpool, PR_NOWAIT);
		if (nbp == NULL) {
			/*
			 * No resources to run this request; leave the
			 * buffer queued up, and schedule a timer to
			 * restart the queue in 1/2 a second.
			 */
			disk_unbusy(&sc->sc_dk, 0, bp->b_flags & B_READ);
			callout_schedule(&sc->sc_restart_ch, hz / 2);
			return;
		}

		(void) BUFQ_GET(&sc->sc_bufq);

		BUF_INIT(nbp);
		nbp->b_data = bp->b_data;
		nbp->b_flags = bp->b_flags | B_CALL;
		nbp->b_iodone = dkiodone;
		nbp->b_proc = bp->b_proc;
		nbp->b_blkno = bp->b_rawblkno;
		nbp->b_dev = sc->sc_parent->dk_rawvp->v_rdev;
		nbp->b_vp = sc->sc_parent->dk_rawvp;
		nbp->b_bcount = bp->b_bcount;
		nbp->b_private = bp;
		BIO_COPYPRIO(nbp, bp);

		if ((nbp->b_flags & B_READ) == 0)
			V_INCR_NUMOUTPUT(nbp->b_vp);
		VOP_STRATEGY(nbp->b_vp, nbp);
	}
}

/*
 * dkiodone:
 *
 *	I/O to a wedge has completed; alert the top half.
 *	NOTE: Must be called at splbio()!
 */
static void
dkiodone(struct buf *bp)
{
	struct buf *obp = bp->b_private;
	struct dkwedge_softc *sc = dkwedge_lookup(obp->b_dev);

	if (bp->b_flags & B_ERROR) {
		obp->b_flags |= B_ERROR;
		obp->b_error = bp->b_error;
	}
	obp->b_resid = bp->b_resid;
	pool_put(&bufpool, bp);

	if (sc->sc_iopend-- == 1 && (sc->sc_flags & DK_F_WAIT_DRAIN) != 0) {
		sc->sc_flags &= ~DK_F_WAIT_DRAIN;
		wakeup(&sc->sc_iopend);
	}

	disk_unbusy(&sc->sc_dk, obp->b_bcount - obp->b_resid,
	    obp->b_flags & B_READ);

	biodone(obp);

	/* Kick the queue in case there is more work we can do. */
	dkstart(sc);
}

/*
 * dkrestart:
 *
 *	Restart the work queue after it was stalled due to
 *	a resource shortage.  Invoked via a callout.
 */
static void
dkrestart(void *v)
{
	struct dkwedge_softc *sc = v;
	int s;

	s = splbio();
	dkstart(sc);
	splx(s);
}

/*
 * dkread:		[devsw entry point]
 *
 *	Read from a wedge.
 */
static int
dkread(dev_t dev, struct uio *uio, int flags)
{
	struct dkwedge_softc *sc = dkwedge_lookup(dev);

	if (sc->sc_state != DKW_STATE_RUNNING)
		return (ENXIO);

	return (physio(dkstrategy, NULL, dev, B_READ,
		       sc->sc_parent->dk_driver->d_minphys, uio));
}

/*
 * dkwrite:		[devsw entry point]
 *
 *	Write to a wedge.
 */
static int
dkwrite(dev_t dev, struct uio *uio, int flags)
{
	struct dkwedge_softc *sc = dkwedge_lookup(dev);

	if (sc->sc_state != DKW_STATE_RUNNING)
		return (ENXIO);

	return (physio(dkstrategy, NULL, dev, B_WRITE,
		       sc->sc_parent->dk_driver->d_minphys, uio));
}

/*
 * dkioctl:		[devsw entry point]
 *
 *	Perform an ioctl request on a wedge.
 */
static int
dkioctl(dev_t dev, u_long cmd, caddr_t data, int flag, struct proc *p)
{
	struct dkwedge_softc *sc = dkwedge_lookup(dev);
	int error = 0;

	if (sc->sc_state != DKW_STATE_RUNNING)
		return (ENXIO);

	switch (cmd) {
	case DIOCCACHESYNC:
		/*
		 * XXX Do we really need to care about having a writable
		 * file descriptor here?
		 */
		if ((flag & FWRITE) == 0)
			error = EBADF;
		else
			error = VOP_IOCTL(sc->sc_parent->dk_rawvp,
					  cmd, data, flag,
					  p != NULL ? p->p_ucred : NOCRED, p);
		break;
	case DIOCGWEDGEINFO:
	    {
	    	struct dkwedge_info *dkw = (void *) data;

		strcpy(dkw->dkw_devname, sc->sc_dev->dv_xname);
	    	memcpy(dkw->dkw_wname, sc->sc_wname, sizeof(dkw->dkw_wname));
		dkw->dkw_wname[sizeof(dkw->dkw_wname) - 1] = '\0';
		strcpy(dkw->dkw_parent, sc->sc_parent->dk_name);
		dkw->dkw_offset = sc->sc_offset;
		dkw->dkw_size = sc->sc_size;
		strcpy(dkw->dkw_ptype, sc->sc_ptype);

		break;
	    }

	default:
		error = ENOTTY;
	}

	return (error);
}

/*
 * dksize:		[devsw entry point]
 *
 *	Query the size of a wedge for the purpose of performing a dump
 *	or for swapping to.
 */
static int
dksize(dev_t dev)
{

	/* XXX */
	return (-1);
}

/*
 * dkdump:		[devsw entry point]
 *
 *	Perform a crash dump to a wedge.
 */
static int
dkdump(dev_t dev, daddr_t blkno, caddr_t va, size_t size)
{

	/* XXX */
	return (ENXIO);
}
