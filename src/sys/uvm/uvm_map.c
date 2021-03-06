/*	$NetBSD: uvm_map.c,v 1.186.2.2 2005/05/01 11:05:06 tron Exp $	*/

/*
 * Copyright (c) 1997 Charles D. Cranor and Washington University.
 * Copyright (c) 1991, 1993, The Regents of the University of California.
 *
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * The Mach Operating System project at Carnegie-Mellon University.
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
 *	This product includes software developed by Charles D. Cranor,
 *      Washington University, the University of California, Berkeley and
 *      its contributors.
 * 4. Neither the name of the University nor the names of its contributors
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
 *	@(#)vm_map.c    8.3 (Berkeley) 1/12/94
 * from: Id: uvm_map.c,v 1.1.2.27 1998/02/07 01:16:54 chs Exp
 *
 *
 * Copyright (c) 1987, 1990 Carnegie-Mellon University.
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software and
 * its documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */

/*
 * uvm_map.c: uvm map operations
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: uvm_map.c,v 1.186.2.2 2005/05/01 11:05:06 tron Exp $");

#include "opt_ddb.h"
#include "opt_uvmhist.h"
#include "opt_uvm.h"
#include "opt_sysv.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mman.h>
#include <sys/proc.h>
#include <sys/malloc.h>
#include <sys/pool.h>
#include <sys/kernel.h>
#include <sys/mount.h>
#include <sys/vnode.h>

#ifdef SYSVSHM
#include <sys/shm.h>
#endif

#define UVM_MAP
#include <uvm/uvm.h>
#undef RB_AUGMENT
#define	RB_AUGMENT(x)	uvm_rb_augment(x)

#ifdef DDB
#include <uvm/uvm_ddb.h>
#endif

#ifndef UVMMAP_NOCOUNTERS
#include <sys/device.h>
struct evcnt map_ubackmerge = EVCNT_INITIALIZER(EVCNT_TYPE_MISC, NULL,
    "uvmmap", "ubackmerge");
struct evcnt map_uforwmerge = EVCNT_INITIALIZER(EVCNT_TYPE_MISC, NULL,
    "uvmmap", "uforwmerge");
struct evcnt map_ubimerge = EVCNT_INITIALIZER(EVCNT_TYPE_MISC, NULL,
    "uvmmap", "ubimerge");
struct evcnt map_unomerge = EVCNT_INITIALIZER(EVCNT_TYPE_MISC, NULL,
    "uvmmap", "unomerge");
struct evcnt map_kbackmerge = EVCNT_INITIALIZER(EVCNT_TYPE_MISC, NULL,
    "uvmmap", "kbackmerge");
struct evcnt map_kforwmerge = EVCNT_INITIALIZER(EVCNT_TYPE_MISC, NULL,
    "uvmmap", "kforwmerge");
struct evcnt map_kbimerge = EVCNT_INITIALIZER(EVCNT_TYPE_MISC, NULL,
    "uvmmap", "kbimerge");
struct evcnt map_knomerge = EVCNT_INITIALIZER(EVCNT_TYPE_MISC, NULL,
    "uvmmap", "knomerge");
struct evcnt uvm_map_call = EVCNT_INITIALIZER(EVCNT_TYPE_MISC, NULL,
    "uvmmap", "map_call");
struct evcnt uvm_mlk_call = EVCNT_INITIALIZER(EVCNT_TYPE_MISC, NULL,
    "uvmmap", "mlk_call");
struct evcnt uvm_mlk_hint = EVCNT_INITIALIZER(EVCNT_TYPE_MISC, NULL,
    "uvmmap", "mlk_hint");

EVCNT_ATTACH_STATIC(map_ubackmerge);
EVCNT_ATTACH_STATIC(map_uforwmerge);
EVCNT_ATTACH_STATIC(map_ubimerge);
EVCNT_ATTACH_STATIC(map_unomerge);
EVCNT_ATTACH_STATIC(map_kbackmerge);
EVCNT_ATTACH_STATIC(map_kforwmerge);
EVCNT_ATTACH_STATIC(map_kbimerge);
EVCNT_ATTACH_STATIC(map_knomerge);
EVCNT_ATTACH_STATIC(uvm_map_call);
EVCNT_ATTACH_STATIC(uvm_mlk_call);
EVCNT_ATTACH_STATIC(uvm_mlk_hint);

#define UVMCNT_INCR(ev)		ev.ev_count++
#define UVMCNT_DECR(ev)		ev.ev_count--
#else
#define UVMCNT_INCR(ev)
#define UVMCNT_DECR(ev)
#endif

const char vmmapbsy[] = "vmmapbsy";

/*
 * pool for vmspace structures.
 */

POOL_INIT(uvm_vmspace_pool, sizeof(struct vmspace), 0, 0, 0, "vmsppl",
    &pool_allocator_nointr);

/*
 * pool for dynamically-allocated map entries.
 */

POOL_INIT(uvm_map_entry_pool, sizeof(struct vm_map_entry), 0, 0, 0, "vmmpepl",
    &pool_allocator_nointr);

MALLOC_DEFINE(M_VMMAP, "VM map", "VM map structures");
MALLOC_DEFINE(M_VMPMAP, "VM pmap", "VM pmap");

#ifdef PMAP_GROWKERNEL
/*
 * This global represents the end of the kernel virtual address
 * space.  If we want to exceed this, we must grow the kernel
 * virtual address space dynamically.
 *
 * Note, this variable is locked by kernel_map's lock.
 */
vaddr_t uvm_maxkaddr;
#endif

/*
 * macros
 */

/*
 * VM_MAP_USE_KMAPENT: determine if uvm_kmapent_alloc/free is used
 * for the vm_map.
 */
extern struct vm_map *pager_map; /* XXX */
#define	VM_MAP_USE_KMAPENT(map) \
	(((map)->flags & VM_MAP_INTRSAFE) || (map) == kernel_map)

/*
 * uvm_map_entry_link: insert entry into a map
 *
 * => map must be locked
 */
#define uvm_map_entry_link(map, after_where, entry) do { \
	KASSERT(entry->start < entry->end); \
	(map)->nentries++; \
	(entry)->prev = (after_where); \
	(entry)->next = (after_where)->next; \
	(entry)->prev->next = (entry); \
	(entry)->next->prev = (entry); \
	uvm_rb_insert((map), (entry)); \
} while (/*CONSTCOND*/ 0)

/*
 * uvm_map_entry_unlink: remove entry from a map
 *
 * => map must be locked
 */
#define uvm_map_entry_unlink(map, entry) do { \
	(map)->nentries--; \
	(entry)->next->prev = (entry)->prev; \
	(entry)->prev->next = (entry)->next; \
	uvm_rb_remove((map), (entry)); \
} while (/*CONSTCOND*/ 0)

/*
 * SAVE_HINT: saves the specified entry as the hint for future lookups.
 *
 * => map need not be locked (protected by hint_lock).
 */
#define SAVE_HINT(map,check,value) do { \
	simple_lock(&(map)->hint_lock); \
	if ((map)->hint == (check)) \
		(map)->hint = (value); \
	simple_unlock(&(map)->hint_lock); \
} while (/*CONSTCOND*/ 0)

/*
 * VM_MAP_RANGE_CHECK: check and correct range
 *
 * => map must at least be read locked
 */

#define VM_MAP_RANGE_CHECK(map, start, end) do { \
	if (start < vm_map_min(map))		\
		start = vm_map_min(map);	\
	if (end > vm_map_max(map))		\
		end = vm_map_max(map);		\
	if (start > end)			\
		start = end;			\
} while (/*CONSTCOND*/ 0)

/*
 * local prototypes
 */

static struct vm_map_entry *
		uvm_mapent_alloc(struct vm_map *, int);
static struct vm_map_entry *
		uvm_mapent_alloc_split(struct vm_map *,
		    const struct vm_map_entry *, int,
		    struct uvm_mapent_reservation *);
static void	uvm_mapent_copy(struct vm_map_entry *, struct vm_map_entry *);
static void	uvm_mapent_free(struct vm_map_entry *);
static struct vm_map_entry *
		uvm_kmapent_alloc(struct vm_map *, int);
static void	uvm_kmapent_free(struct vm_map_entry *);
static void	uvm_map_entry_unwire(struct vm_map *, struct vm_map_entry *);
static void	uvm_map_reference_amap(struct vm_map_entry *, int);
static int	uvm_map_space_avail(vaddr_t *, vsize_t, voff_t, vsize_t, int,
		    struct vm_map_entry *);
static void	uvm_map_unreference_amap(struct vm_map_entry *, int);

int _uvm_tree_sanity(struct vm_map *, const char *);
static vsize_t uvm_rb_subtree_space(const struct vm_map_entry *);

static __inline int
uvm_compare(const struct vm_map_entry *a, const struct vm_map_entry *b)
{

	if (a->start < b->start)
		return (-1);
	else if (a->start > b->start)
		return (1);

	return (0);
}

static __inline void
uvm_rb_augment(struct vm_map_entry *entry)
{

	entry->space = uvm_rb_subtree_space(entry);
}

RB_PROTOTYPE(uvm_tree, vm_map_entry, rb_entry, uvm_compare);

RB_GENERATE(uvm_tree, vm_map_entry, rb_entry, uvm_compare);

static __inline vsize_t
uvm_rb_space(const struct vm_map *map, const struct vm_map_entry *entry)
{
	/* XXX map is not used */

	KASSERT(entry->next != NULL);
	return entry->next->start - entry->end;
}

static vsize_t
uvm_rb_subtree_space(const struct vm_map_entry *entry)
{
	vaddr_t space, tmp;

	space = entry->ownspace;
	if (RB_LEFT(entry, rb_entry)) {
		tmp = RB_LEFT(entry, rb_entry)->space;
		if (tmp > space)
			space = tmp;
	}

	if (RB_RIGHT(entry, rb_entry)) {
		tmp = RB_RIGHT(entry, rb_entry)->space;
		if (tmp > space)
			space = tmp;
	}

	return (space);
}

static __inline void
uvm_rb_fixup(struct vm_map *map, struct vm_map_entry *entry)
{
	/* We need to traverse to the very top */
	do {
		entry->ownspace = uvm_rb_space(map, entry);
		entry->space = uvm_rb_subtree_space(entry);
	} while ((entry = RB_PARENT(entry, rb_entry)) != NULL);
}

static __inline void
uvm_rb_insert(struct vm_map *map, struct vm_map_entry *entry)
{
	vaddr_t space = uvm_rb_space(map, entry);
	struct vm_map_entry *tmp;

	entry->ownspace = entry->space = space;
	tmp = RB_INSERT(uvm_tree, &(map)->rbhead, entry);
#ifdef DIAGNOSTIC
	if (tmp != NULL)
		panic("uvm_rb_insert: duplicate entry?");
#endif
	uvm_rb_fixup(map, entry);
	if (entry->prev != &map->header)
		uvm_rb_fixup(map, entry->prev);
}

static __inline void
uvm_rb_remove(struct vm_map *map, struct vm_map_entry *entry)
{
	struct vm_map_entry *parent;

	parent = RB_PARENT(entry, rb_entry);
	RB_REMOVE(uvm_tree, &(map)->rbhead, entry);
	if (entry->prev != &map->header)
		uvm_rb_fixup(map, entry->prev);
	if (parent)
		uvm_rb_fixup(map, parent);
}

#ifdef DEBUG
int uvm_debug_check_rbtree = 0;
#define uvm_tree_sanity(x,y)		\
	if (uvm_debug_check_rbtree)	\
		_uvm_tree_sanity(x,y)
#else
#define uvm_tree_sanity(x,y)
#endif

int
_uvm_tree_sanity(struct vm_map *map, const char *name)
{
	struct vm_map_entry *tmp, *trtmp;
	int n = 0, i = 1;

	RB_FOREACH(tmp, uvm_tree, &map->rbhead) {
		if (tmp->ownspace != uvm_rb_space(map, tmp)) {
			printf("%s: %d/%d ownspace %lx != %lx %s\n",
			    name, n + 1, map->nentries,
			    (ulong)tmp->ownspace, (ulong)uvm_rb_space(map, tmp),
			    tmp->next == &map->header ? "(last)" : "");
			goto error;
		}
	}
	trtmp = NULL;
	RB_FOREACH(tmp, uvm_tree, &map->rbhead) {
		if (tmp->space != uvm_rb_subtree_space(tmp)) {
			printf("%s: space %lx != %lx\n",
			    name, (ulong)tmp->space,
			    (ulong)uvm_rb_subtree_space(tmp));
			goto error;
		}
		if (trtmp != NULL && trtmp->start >= tmp->start) {
			printf("%s: corrupt: 0x%lx >= 0x%lx\n",
			    name, trtmp->start, tmp->start);
			goto error;
		}
		n++;

		trtmp = tmp;
	}

	if (n != map->nentries) {
		printf("%s: nentries: %d vs %d\n",
		    name, n, map->nentries);
		goto error;
	}

	for (tmp = map->header.next; tmp && tmp != &map->header;
	    tmp = tmp->next, i++) {
		trtmp = RB_FIND(uvm_tree, &map->rbhead, tmp);
		if (trtmp != tmp) {
			printf("%s: lookup: %d: %p - %p: %p\n",
			    name, i, tmp, trtmp,
			    RB_PARENT(tmp, rb_entry));
			goto error;
		}
	}

	return (0);
 error:
#ifdef	DDB
	/* handy breakpoint location for error case */
	__asm(".globl treesanity_label\ntreesanity_label:");
#endif
	return (-1);
}

/*
 * local inlines
 */

static __inline struct vm_map *uvm_kmapent_map(struct vm_map_entry *);

/*
 * uvm_mapent_alloc: allocate a map entry
 */

static __inline struct vm_map_entry *
uvm_mapent_alloc(struct vm_map *map, int flags)
{
	struct vm_map_entry *me;
	int pflags = (flags & UVM_FLAG_NOWAIT) ? PR_NOWAIT : PR_WAITOK;
	UVMHIST_FUNC("uvm_mapent_alloc"); UVMHIST_CALLED(maphist);

	if (VM_MAP_USE_KMAPENT(map)) {
		me = uvm_kmapent_alloc(map, flags);
	} else {
		me = pool_get(&uvm_map_entry_pool, pflags);
		if (__predict_false(me == NULL))
			return NULL;
		me->flags = 0;
	}

	UVMHIST_LOG(maphist, "<- new entry=0x%x [kentry=%d]", me,
	    ((map->flags & VM_MAP_INTRSAFE) != 0 || map == kernel_map), 0, 0);
	return (me);
}

/*
 * uvm_mapent_alloc_split: allocate a map entry for clipping.
 */

static __inline struct vm_map_entry *
uvm_mapent_alloc_split(struct vm_map *map,
    const struct vm_map_entry *old_entry, int flags,
    struct uvm_mapent_reservation *umr)
{
	struct vm_map_entry *me;

	KASSERT(!VM_MAP_USE_KMAPENT(map) ||
	    (old_entry->flags & UVM_MAP_QUANTUM) || !UMR_EMPTY(umr));

	if (old_entry->flags & UVM_MAP_QUANTUM) {
		int s;
		struct vm_map_kernel *vmk = vm_map_to_kernel(map);

		s = splvm();
		simple_lock(&uvm.kentry_lock);
		me = vmk->vmk_merged_entries;
		KASSERT(me);
		vmk->vmk_merged_entries = me->next;
		simple_unlock(&uvm.kentry_lock);
		splx(s);
		KASSERT(me->flags & UVM_MAP_QUANTUM);
	} else {
		me = uvm_mapent_alloc(map, flags);
	}

	return me;
}

/*
 * uvm_mapent_free: free map entry
 */

static __inline void
uvm_mapent_free(struct vm_map_entry *me)
{
	UVMHIST_FUNC("uvm_mapent_free"); UVMHIST_CALLED(maphist);

	UVMHIST_LOG(maphist,"<- freeing map entry=0x%x [flags=%d]",
		me, me->flags, 0, 0);
	if (me->flags & UVM_MAP_KERNEL) {
		uvm_kmapent_free(me);
	} else {
		pool_put(&uvm_map_entry_pool, me);
	}
}

/*
 * uvm_mapent_free_merge: free merged map entry
 *
 * => keep the entry if needed.
 * => caller shouldn't hold map locked.
 */

static __inline void
uvm_mapent_free_merged(struct vm_map *map, struct vm_map_entry *me)
{

	KASSERT(!(me->flags & UVM_MAP_KERNEL) || uvm_kmapent_map(me) == map);

	if (me->flags & UVM_MAP_QUANTUM) {
		/*
		 * keep this entry for later splitting.
		 */
		struct vm_map_kernel *vmk;
		int s;

		KASSERT(VM_MAP_IS_KERNEL(map));
		KASSERT(!VM_MAP_USE_KMAPENT(map) ||
		    (me->flags & UVM_MAP_KERNEL));

		vmk = vm_map_to_kernel(map);
		s = splvm();
		simple_lock(&uvm.kentry_lock);
		me->next = vmk->vmk_merged_entries;
		vmk->vmk_merged_entries = me;
		simple_unlock(&uvm.kentry_lock);
		splx(s);
	} else {
		uvm_mapent_free(me);
	}
}

/*
 * uvm_mapent_copy: copy a map entry, preserving flags
 */

static __inline void
uvm_mapent_copy(struct vm_map_entry *src, struct vm_map_entry *dst)
{

	memcpy(dst, src, ((char *)&src->uvm_map_entry_stop_copy) -
	    ((char *)src));
}

/*
 * uvm_map_entry_unwire: unwire a map entry
 *
 * => map should be locked by caller
 */

static __inline void
uvm_map_entry_unwire(struct vm_map *map, struct vm_map_entry *entry)
{

	entry->wired_count = 0;
	uvm_fault_unwire_locked(map, entry->start, entry->end);
}


/*
 * wrapper for calling amap_ref()
 */
static __inline void
uvm_map_reference_amap(struct vm_map_entry *entry, int flags)
{

	amap_ref(entry->aref.ar_amap, entry->aref.ar_pageoff,
	    (entry->end - entry->start) >> PAGE_SHIFT, flags);
}


/*
 * wrapper for calling amap_unref()
 */
static __inline void
uvm_map_unreference_amap(struct vm_map_entry *entry, int flags)
{

	amap_unref(entry->aref.ar_amap, entry->aref.ar_pageoff,
	    (entry->end - entry->start) >> PAGE_SHIFT, flags);
}


/*
 * uvm_map_init: init mapping system at boot time.   note that we allocate
 * and init the static pool of struct vm_map_entry *'s for the kernel here.
 */

void
uvm_map_init(void)
{
#if defined(UVMHIST)
	static struct uvm_history_ent maphistbuf[100];
	static struct uvm_history_ent pdhistbuf[100];
#endif

	/*
	 * first, init logging system.
	 */

	UVMHIST_FUNC("uvm_map_init");
	UVMHIST_INIT_STATIC(maphist, maphistbuf);
	UVMHIST_INIT_STATIC(pdhist, pdhistbuf);
	UVMHIST_CALLED(maphist);
	UVMHIST_LOG(maphist,"<starting uvm map system>", 0, 0, 0, 0);

	/*
	 * initialize the global lock for kernel map entry.
	 *
	 * XXX is it worth to have per-map lock instead?
	 */

	simple_lock_init(&uvm.kentry_lock);
}

/*
 * clippers
 */

/*
 * uvm_map_clip_start: ensure that the entry begins at or after
 *	the starting address, if it doesn't we split the entry.
 *
 * => caller should use UVM_MAP_CLIP_START macro rather than calling
 *    this directly
 * => map must be locked by caller
 */

void
uvm_map_clip_start(struct vm_map *map, struct vm_map_entry *entry,
    vaddr_t start, struct uvm_mapent_reservation *umr)
{
	struct vm_map_entry *new_entry;
	vaddr_t new_adj;

	/* uvm_map_simplify_entry(map, entry); */ /* XXX */

	uvm_tree_sanity(map, "clip_start entry");

	/*
	 * Split off the front portion.  note that we must insert the new
	 * entry BEFORE this one, so that this entry has the specified
	 * starting address.
	 */
	new_entry = uvm_mapent_alloc_split(map, entry, 0, umr);
	uvm_mapent_copy(entry, new_entry); /* entry -> new_entry */

	new_entry->end = start;
	new_adj = start - new_entry->start;
	if (entry->object.uvm_obj)
		entry->offset += new_adj;	/* shift start over */

	/* Does not change order for the RB tree */
	entry->start = start;

	if (new_entry->aref.ar_amap) {
		amap_splitref(&new_entry->aref, &entry->aref, new_adj);
	}

	uvm_map_entry_link(map, entry->prev, new_entry);

	if (UVM_ET_ISSUBMAP(entry)) {
		/* ... unlikely to happen, but play it safe */
		 uvm_map_reference(new_entry->object.sub_map);
	} else {
		if (UVM_ET_ISOBJ(entry) &&
		    entry->object.uvm_obj->pgops &&
		    entry->object.uvm_obj->pgops->pgo_reference)
			entry->object.uvm_obj->pgops->pgo_reference(
			    entry->object.uvm_obj);
	}

	uvm_tree_sanity(map, "clip_start leave");
}

/*
 * uvm_map_clip_end: ensure that the entry ends at or before
 *	the ending address, if it does't we split the reference
 *
 * => caller should use UVM_MAP_CLIP_END macro rather than calling
 *    this directly
 * => map must be locked by caller
 */

void
uvm_map_clip_end(struct vm_map *map, struct vm_map_entry *entry, vaddr_t end,
    struct uvm_mapent_reservation *umr)
{
	struct vm_map_entry *	new_entry;
	vaddr_t new_adj; /* #bytes we move start forward */

	uvm_tree_sanity(map, "clip_end entry");

	/*
	 *	Create a new entry and insert it
	 *	AFTER the specified entry
	 */
	new_entry = uvm_mapent_alloc_split(map, entry, 0, umr);
	uvm_mapent_copy(entry, new_entry); /* entry -> new_entry */

	new_entry->start = entry->end = end;
	new_adj = end - entry->start;
	if (new_entry->object.uvm_obj)
		new_entry->offset += new_adj;

	if (entry->aref.ar_amap)
		amap_splitref(&entry->aref, &new_entry->aref, new_adj);

	uvm_rb_fixup(map, entry);

	uvm_map_entry_link(map, entry, new_entry);

	if (UVM_ET_ISSUBMAP(entry)) {
		/* ... unlikely to happen, but play it safe */
		uvm_map_reference(new_entry->object.sub_map);
	} else {
		if (UVM_ET_ISOBJ(entry) &&
		    entry->object.uvm_obj->pgops &&
		    entry->object.uvm_obj->pgops->pgo_reference)
			entry->object.uvm_obj->pgops->pgo_reference(
			    entry->object.uvm_obj);
	}

	uvm_tree_sanity(map, "clip_end leave");
}


/*
 *   M A P   -   m a i n   e n t r y   p o i n t
 */
/*
 * uvm_map: establish a valid mapping in a map
 *
 * => assume startp is page aligned.
 * => assume size is a multiple of PAGE_SIZE.
 * => assume sys_mmap provides enough of a "hint" to have us skip
 *	over text/data/bss area.
 * => map must be unlocked (we will lock it)
 * => <uobj,uoffset> value meanings (4 cases):
 *	 [1] <NULL,uoffset>		== uoffset is a hint for PMAP_PREFER
 *	 [2] <NULL,UVM_UNKNOWN_OFFSET>	== don't PMAP_PREFER
 *	 [3] <uobj,uoffset>		== normal mapping
 *	 [4] <uobj,UVM_UNKNOWN_OFFSET>	== uvm_map finds offset based on VA
 *
 *    case [4] is for kernel mappings where we don't know the offset until
 *    we've found a virtual address.   note that kernel object offsets are
 *    always relative to vm_map_min(kernel_map).
 *
 * => if `align' is non-zero, we align the virtual address to the specified
 *	alignment.
 *	this is provided as a mechanism for large pages.
 *
 * => XXXCDC: need way to map in external amap?
 */

int
uvm_map(struct vm_map *map, vaddr_t *startp /* IN/OUT */, vsize_t size,
    struct uvm_object *uobj, voff_t uoffset, vsize_t align, uvm_flag_t flags)
{
	struct uvm_map_args args;
	struct vm_map_entry *new_entry;
	int error;

	KASSERT((flags & UVM_FLAG_QUANTUM) == 0 || VM_MAP_IS_KERNEL(map));

	/*
	 * for pager_map, allocate the new entry first to avoid sleeping
	 * for memory while we have the map locked.
	 *
	 * besides, because we allocates entries for in-kernel maps
	 * a bit differently (cf. uvm_kmapent_alloc/free), we need to
	 * allocate them before locking the map.
	 */

	new_entry = NULL;
	if (VM_MAP_USE_KMAPENT(map) || (flags & UVM_FLAG_QUANTUM) ||
	    map == pager_map) {
		new_entry = uvm_mapent_alloc(map, (flags & UVM_FLAG_NOWAIT));
		if (__predict_false(new_entry == NULL))
			return ENOMEM;
		if (flags & UVM_FLAG_QUANTUM)
			new_entry->flags |= UVM_MAP_QUANTUM;
	}
	if (map == pager_map)
		flags |= UVM_FLAG_NOMERGE;

	error = uvm_map_prepare(map, *startp, size, uobj, uoffset, align,
	    flags, &args);
	if (!error) {
		error = uvm_map_enter(map, &args, new_entry);
		*startp = args.uma_start;
	} else if (new_entry) {
		uvm_mapent_free(new_entry);
	}

	return error;
}

int
uvm_map_prepare(struct vm_map *map, vaddr_t start, vsize_t size,
    struct uvm_object *uobj, voff_t uoffset, vsize_t align, uvm_flag_t flags,
    struct uvm_map_args *args)
{
	struct vm_map_entry *prev_entry;
	vm_prot_t prot = UVM_PROTECTION(flags);
	vm_prot_t maxprot = UVM_MAXPROTECTION(flags);

	UVMHIST_FUNC("uvm_map_prepare");
	UVMHIST_CALLED(maphist);

	UVMHIST_LOG(maphist, "(map=0x%x, start=0x%x, size=%d, flags=0x%x)",
	    map, start, size, flags);
	UVMHIST_LOG(maphist, "  uobj/offset 0x%x/%d", uobj, uoffset,0,0);

	/*
	 * detect a popular device driver bug.
	 */

	KASSERT(doing_shutdown || curlwp != NULL ||
	    (map->flags & VM_MAP_INTRSAFE));

	/*
	 * zero-sized mapping doesn't make any sense.
	 */
	KASSERT(size > 0);

	KASSERT((~flags & (UVM_FLAG_NOWAIT | UVM_FLAG_WAITVA)) != 0);

	uvm_tree_sanity(map, "map entry");

	/*
	 * check sanity of protection code
	 */

	if ((prot & maxprot) != prot) {
		UVMHIST_LOG(maphist, "<- prot. failure:  prot=0x%x, max=0x%x",
		prot, maxprot,0,0);
		return EACCES;
	}

	/*
	 * figure out where to put new VM range
	 */

retry:
	if (vm_map_lock_try(map) == FALSE) {
		if (flags & UVM_FLAG_TRYLOCK) {
			return EAGAIN;
		}
		vm_map_lock(map); /* could sleep here */
	}
	if ((prev_entry = uvm_map_findspace(map, start, size, &start,
	    uobj, uoffset, align, flags)) == NULL) {
		unsigned int timestamp;

		if ((flags & UVM_FLAG_WAITVA) == 0) {
			UVMHIST_LOG(maphist,"<- uvm_map_findspace failed!",
			    0,0,0,0);
			vm_map_unlock(map);
			return ENOMEM;
		}
		timestamp = map->timestamp;
		UVMHIST_LOG(maphist,"waiting va timestamp=0x%x",
			    timestamp,0,0,0);
		simple_lock(&map->flags_lock);
		map->flags |= VM_MAP_WANTVA;
		simple_unlock(&map->flags_lock);
		vm_map_unlock(map);

		/*
		 * wait until someone does unmap.
		 * XXX fragile locking
		 */

		simple_lock(&map->flags_lock);
		while ((map->flags & VM_MAP_WANTVA) != 0 &&
		   map->timestamp == timestamp) {
			ltsleep(&map->header, PVM, "vmmapva", 0,
			    &map->flags_lock);
		}
		simple_unlock(&map->flags_lock);
		goto retry;
	}

#ifdef PMAP_GROWKERNEL
	/*
	 * If the kernel pmap can't map the requested space,
	 * then allocate more resources for it.
	 */
	if (map == kernel_map && uvm_maxkaddr < (start + size))
		uvm_maxkaddr = pmap_growkernel(start + size);
#endif

	UVMCNT_INCR(uvm_map_call);

	/*
	 * if uobj is null, then uoffset is either a VAC hint for PMAP_PREFER
	 * [typically from uvm_map_reserve] or it is UVM_UNKNOWN_OFFSET.   in
	 * either case we want to zero it  before storing it in the map entry
	 * (because it looks strange and confusing when debugging...)
	 *
	 * if uobj is not null
	 *   if uoffset is not UVM_UNKNOWN_OFFSET then we have a normal mapping
	 *      and we do not need to change uoffset.
	 *   if uoffset is UVM_UNKNOWN_OFFSET then we need to find the offset
	 *      now (based on the starting address of the map).   this case is
	 *      for kernel object mappings where we don't know the offset until
	 *      the virtual address is found (with uvm_map_findspace).   the
	 *      offset is the distance we are from the start of the map.
	 */

	if (uobj == NULL) {
		uoffset = 0;
	} else {
		if (uoffset == UVM_UNKNOWN_OFFSET) {
			KASSERT(UVM_OBJ_IS_KERN_OBJECT(uobj));
			uoffset = start - vm_map_min(kernel_map);
		}
	}

	args->uma_flags = flags;
	args->uma_prev = prev_entry;
	args->uma_start = start;
	args->uma_size = size;
	args->uma_uobj = uobj;
	args->uma_uoffset = uoffset;

	return 0;
}

int
uvm_map_enter(struct vm_map *map, const struct uvm_map_args *args,
    struct vm_map_entry *new_entry)
{
	struct vm_map_entry *prev_entry = args->uma_prev;
	struct vm_map_entry *dead = NULL;

	const uvm_flag_t flags = args->uma_flags;
	const vm_prot_t prot = UVM_PROTECTION(flags);
	const vm_prot_t maxprot = UVM_MAXPROTECTION(flags);
	const vm_inherit_t inherit = UVM_INHERIT(flags);
	const int amapwaitflag = (flags & UVM_FLAG_NOWAIT) ?
	    AMAP_EXTEND_NOWAIT : 0;
	const int advice = UVM_ADVICE(flags);
	const int meflagmask = UVM_MAP_NOMERGE | UVM_MAP_QUANTUM;
	const int meflagval = (flags & UVM_FLAG_QUANTUM) ?
	    UVM_MAP_QUANTUM : 0;

	vaddr_t start = args->uma_start;
	vsize_t size = args->uma_size;
	struct uvm_object *uobj = args->uma_uobj;
	voff_t uoffset = args->uma_uoffset;

	const int kmap = (vm_map_pmap(map) == pmap_kernel());
	int merged = 0;
	int error;
	int newetype;

	UVMHIST_FUNC("uvm_map_enter");
	UVMHIST_CALLED(maphist);

	UVMHIST_LOG(maphist, "(map=0x%x, start=0x%x, size=%d, flags=0x%x)",
	    map, start, size, flags);
	UVMHIST_LOG(maphist, "  uobj/offset 0x%x/%d", uobj, uoffset,0,0);

	if (flags & UVM_FLAG_QUANTUM) {
		KASSERT(new_entry);
		KASSERT(new_entry->flags & UVM_MAP_QUANTUM);
	}

	if (uobj)
		newetype = UVM_ET_OBJ;
	else
		newetype = 0;

	if (flags & UVM_FLAG_COPYONW) {
		newetype |= UVM_ET_COPYONWRITE;
		if ((flags & UVM_FLAG_OVERLAY) == 0)
			newetype |= UVM_ET_NEEDSCOPY;
	}

	/*
	 * try and insert in map by extending previous entry, if possible.
	 * XXX: we don't try and pull back the next entry.   might be useful
	 * for a stack, but we are currently allocating our stack in advance.
	 */

	if (flags & UVM_FLAG_NOMERGE)
		goto nomerge;

	if (prev_entry->etype == newetype &&
	    prev_entry->end == start &&
	    prev_entry != &map->header &&
	    prev_entry->object.uvm_obj == uobj) {

		if ((prev_entry->flags & meflagmask) != meflagval)
			goto forwardmerge;

		if (uobj && prev_entry->offset +
		    (prev_entry->end - prev_entry->start) != uoffset)
			goto forwardmerge;

		if (prev_entry->protection != prot ||
		    prev_entry->max_protection != maxprot)
			goto forwardmerge;

		if (prev_entry->inheritance != inherit ||
		    prev_entry->advice != advice)
			goto forwardmerge;

		/* wiring status must match (new area is unwired) */
		if (VM_MAPENT_ISWIRED(prev_entry))
			goto forwardmerge;

		/*
		 * can't extend a shared amap.  note: no need to lock amap to
		 * look at refs since we don't care about its exact value.
		 * if it is one (i.e. we have only reference) it will stay there
		 */

		if (prev_entry->aref.ar_amap &&
		    amap_refs(prev_entry->aref.ar_amap) != 1) {
			goto forwardmerge;
		}

		if (prev_entry->aref.ar_amap) {
			error = amap_extend(prev_entry, size,
			    amapwaitflag | AMAP_EXTEND_FORWARDS);
			if (error)
				goto done;
		}

		if (kmap)
			UVMCNT_INCR(map_kbackmerge);
		else
			UVMCNT_INCR(map_ubackmerge);
		UVMHIST_LOG(maphist,"  starting back merge", 0, 0, 0, 0);

		/*
		 * drop our reference to uobj since we are extending a reference
		 * that we already have (the ref count can not drop to zero).
		 */

		if (uobj && uobj->pgops->pgo_detach)
			uobj->pgops->pgo_detach(uobj);

		prev_entry->end += size;
		uvm_rb_fixup(map, prev_entry);

		uvm_tree_sanity(map, "map backmerged");

		UVMHIST_LOG(maphist,"<- done (via backmerge)!", 0, 0, 0, 0);
		merged++;
	}

forwardmerge:
	if (prev_entry->next->etype == newetype &&
	    prev_entry->next->start == (start + size) &&
	    prev_entry->next != &map->header &&
	    prev_entry->next->object.uvm_obj == uobj) {

		if ((prev_entry->next->flags & meflagmask) != meflagval)
			goto nomerge;

		if (uobj && prev_entry->next->offset != uoffset + size)
			goto nomerge;

		if (prev_entry->next->protection != prot ||
		    prev_entry->next->max_protection != maxprot)
			goto nomerge;

		if (prev_entry->next->inheritance != inherit ||
		    prev_entry->next->advice != advice)
			goto nomerge;

		/* wiring status must match (new area is unwired) */
		if (VM_MAPENT_ISWIRED(prev_entry->next))
			goto nomerge;

		/*
		 * can't extend a shared amap.  note: no need to lock amap to
		 * look at refs since we don't care about its exact value.
		 * if it is one (i.e. we have only reference) it will stay there.
		 *
		 * note that we also can't merge two amaps, so if we
		 * merged with the previous entry which has an amap,
		 * and the next entry also has an amap, we give up.
		 *
		 * Interesting cases:
		 * amap, new, amap -> give up second merge (single fwd extend)
		 * amap, new, none -> double forward extend (extend again here)
		 * none, new, amap -> double backward extend (done here)
		 * uobj, new, amap -> single backward extend (done here)
		 *
		 * XXX should we attempt to deal with someone refilling
		 * the deallocated region between two entries that are
		 * backed by the same amap (ie, arefs is 2, "prev" and
		 * "next" refer to it, and adding this allocation will
		 * close the hole, thus restoring arefs to 1 and
		 * deallocating the "next" vm_map_entry)?  -- @@@
		 */

		if (prev_entry->next->aref.ar_amap &&
		    (amap_refs(prev_entry->next->aref.ar_amap) != 1 ||
		     (merged && prev_entry->aref.ar_amap))) {
			goto nomerge;
		}

		if (merged) {
			/*
			 * Try to extend the amap of the previous entry to
			 * cover the next entry as well.  If it doesn't work
			 * just skip on, don't actually give up, since we've
			 * already completed the back merge.
			 */
			if (prev_entry->aref.ar_amap) {
				if (amap_extend(prev_entry,
				    prev_entry->next->end -
				    prev_entry->next->start,
				    amapwaitflag | AMAP_EXTEND_FORWARDS))
					goto nomerge;
			}

			/*
			 * Try to extend the amap of the *next* entry
			 * back to cover the new allocation *and* the
			 * previous entry as well (the previous merge
			 * didn't have an amap already otherwise we
			 * wouldn't be checking here for an amap).  If
			 * it doesn't work just skip on, again, don't
			 * actually give up, since we've already
			 * completed the back merge.
			 */
			else if (prev_entry->next->aref.ar_amap) {
				if (amap_extend(prev_entry->next,
				    prev_entry->end -
				    prev_entry->start,
				    amapwaitflag | AMAP_EXTEND_BACKWARDS))
					goto nomerge;
			}
		} else {
			/*
			 * Pull the next entry's amap backwards to cover this
			 * new allocation.
			 */
			if (prev_entry->next->aref.ar_amap) {
				error = amap_extend(prev_entry->next, size,
				    amapwaitflag | AMAP_EXTEND_BACKWARDS);
				if (error)
					goto done;
			}
		}

		if (merged) {
			if (kmap) {
				UVMCNT_DECR(map_kbackmerge);
				UVMCNT_INCR(map_kbimerge);
			} else {
				UVMCNT_DECR(map_ubackmerge);
				UVMCNT_INCR(map_ubimerge);
			}
		} else {
			if (kmap)
				UVMCNT_INCR(map_kforwmerge);
			else
				UVMCNT_INCR(map_uforwmerge);
		}
		UVMHIST_LOG(maphist,"  starting forward merge", 0, 0, 0, 0);

		/*
		 * drop our reference to uobj since we are extending a reference
		 * that we already have (the ref count can not drop to zero).
		 * (if merged, we've already detached)
		 */
		if (uobj && uobj->pgops->pgo_detach && !merged)
			uobj->pgops->pgo_detach(uobj);

		if (merged) {
			dead = prev_entry->next;
			prev_entry->end = dead->end;
			uvm_map_entry_unlink(map, dead);
			if (dead->aref.ar_amap != NULL) {
				prev_entry->aref = dead->aref;
				dead->aref.ar_amap = NULL;
			}
		} else {
			prev_entry->next->start -= size;
			if (prev_entry != &map->header)
				uvm_rb_fixup(map, prev_entry);
			if (uobj)
				prev_entry->next->offset = uoffset;
		}

		uvm_tree_sanity(map, "map forwardmerged");

		UVMHIST_LOG(maphist,"<- done forwardmerge", 0, 0, 0, 0);
		merged++;
	}

nomerge:
	if (!merged) {
		UVMHIST_LOG(maphist,"  allocating new map entry", 0, 0, 0, 0);
		if (kmap)
			UVMCNT_INCR(map_knomerge);
		else
			UVMCNT_INCR(map_unomerge);

		/*
		 * allocate new entry and link it in.
		 */

		if (new_entry == NULL) {
			new_entry = uvm_mapent_alloc(map,
				(flags & UVM_FLAG_NOWAIT));
			if (__predict_false(new_entry == NULL)) {
				error = ENOMEM;
				goto done;
			}
		}
		new_entry->start = start;
		new_entry->end = new_entry->start + size;
		new_entry->object.uvm_obj = uobj;
		new_entry->offset = uoffset;

		new_entry->etype = newetype;

		if (flags & UVM_FLAG_NOMERGE) {
			new_entry->flags |= UVM_MAP_NOMERGE;
		}

		new_entry->protection = prot;
		new_entry->max_protection = maxprot;
		new_entry->inheritance = inherit;
		new_entry->wired_count = 0;
		new_entry->advice = advice;
		if (flags & UVM_FLAG_OVERLAY) {

			/*
			 * to_add: for BSS we overallocate a little since we
			 * are likely to extend
			 */

			vaddr_t to_add = (flags & UVM_FLAG_AMAPPAD) ?
				UVM_AMAP_CHUNK << PAGE_SHIFT : 0;
			struct vm_amap *amap = amap_alloc(size, to_add,
			    (flags & UVM_FLAG_NOWAIT) ? M_NOWAIT : M_WAITOK);
			if (__predict_false(amap == NULL)) {
				error = ENOMEM;
				goto done;
			}
			new_entry->aref.ar_pageoff = 0;
			new_entry->aref.ar_amap = amap;
		} else {
			new_entry->aref.ar_pageoff = 0;
			new_entry->aref.ar_amap = NULL;
		}
		uvm_map_entry_link(map, prev_entry, new_entry);

		/*
		 * Update the free space hint
		 */

		if ((map->first_free == prev_entry) &&
		    (prev_entry->end >= new_entry->start))
			map->first_free = new_entry;

		new_entry = NULL;
	}

	map->size += size;

	UVMHIST_LOG(maphist,"<- done!", 0, 0, 0, 0);

	error = 0;
done:
	vm_map_unlock(map);
	if (new_entry) {
		if (error == 0) {
			KDASSERT(merged);
			uvm_mapent_free_merged(map, new_entry);
		} else {
			uvm_mapent_free(new_entry);
		}
	}
	if (dead) {
		KDASSERT(merged);
		uvm_mapent_free_merged(map, dead);
	}
	return error;
}

/*
 * uvm_map_lookup_entry: find map entry at or before an address
 *
 * => map must at least be read-locked by caller
 * => entry is returned in "entry"
 * => return value is true if address is in the returned entry
 */

boolean_t
uvm_map_lookup_entry(struct vm_map *map, vaddr_t address,
    struct vm_map_entry **entry	/* OUT */)
{
	struct vm_map_entry *cur;
	boolean_t use_tree = FALSE;
	UVMHIST_FUNC("uvm_map_lookup_entry");
	UVMHIST_CALLED(maphist);

	UVMHIST_LOG(maphist,"(map=0x%x,addr=0x%x,ent=0x%x)",
	    map, address, entry, 0);

	/*
	 * start looking either from the head of the
	 * list, or from the hint.
	 */

	simple_lock(&map->hint_lock);
	cur = map->hint;
	simple_unlock(&map->hint_lock);

	if (cur == &map->header)
		cur = cur->next;

	UVMCNT_INCR(uvm_mlk_call);
	if (address >= cur->start) {

		/*
		 * go from hint to end of list.
		 *
		 * but first, make a quick check to see if
		 * we are already looking at the entry we
		 * want (which is usually the case).
		 * note also that we don't need to save the hint
		 * here... it is the same hint (unless we are
		 * at the header, in which case the hint didn't
		 * buy us anything anyway).
		 */

		if (cur != &map->header && cur->end > address) {
			UVMCNT_INCR(uvm_mlk_hint);
			*entry = cur;
			UVMHIST_LOG(maphist,"<- got it via hint (0x%x)",
			    cur, 0, 0, 0);
			return (TRUE);
		}

		if (map->nentries > 30)
			use_tree = TRUE;
	} else {

		/*
		 * invalid hint.  use tree.
		 */
		use_tree = TRUE;
	}

	uvm_tree_sanity(map, __func__);

	if (use_tree) {
		struct vm_map_entry *prev = &map->header;
		cur = RB_ROOT(&map->rbhead);

		/*
		 * Simple lookup in the tree.  Happens when the hint is
		 * invalid, or nentries reach a threshold.
		 */
		while (cur) {
			if (address >= cur->start) {
				if (address < cur->end) {
					*entry = cur;
					goto got;
				}
				prev = cur;
				cur = RB_RIGHT(cur, rb_entry);
			} else
				cur = RB_LEFT(cur, rb_entry);
		}
		*entry = prev;
		goto failed;
	}

	/*
	 * search linearly
	 */

	while (cur != &map->header) {
		if (cur->end > address) {
			if (address >= cur->start) {
				/*
				 * save this lookup for future
				 * hints, and return
				 */

				*entry = cur;
got:
				SAVE_HINT(map, map->hint, *entry);
				UVMHIST_LOG(maphist,"<- search got it (0x%x)",
					cur, 0, 0, 0);
				KDASSERT((*entry)->start <= address);
				KDASSERT(address < (*entry)->end);
				return (TRUE);
			}
			break;
		}
		cur = cur->next;
	}
	*entry = cur->prev;
failed:
	SAVE_HINT(map, map->hint, *entry);
	UVMHIST_LOG(maphist,"<- failed!",0,0,0,0);
	KDASSERT((*entry) == &map->header || (*entry)->end <= address);
	KDASSERT((*entry)->next == &map->header ||
	    address < (*entry)->next->start);
	return (FALSE);
}

/*
 * See if the range between start and start + length fits in the gap
 * entry->next->start and entry->end.  Returns 1 if fits, 0 if doesn't
 * fit, and -1 address wraps around.
 */
static __inline int
uvm_map_space_avail(vaddr_t *start, vsize_t length, voff_t uoffset,
    vsize_t align, int topdown, struct vm_map_entry *entry)
{
	vaddr_t end;

#ifdef PMAP_PREFER
	/*
	 * push start address forward as needed to avoid VAC alias problems.
	 * we only do this if a valid offset is specified.
	 */

	if (uoffset != UVM_UNKNOWN_OFFSET)
		PMAP_PREFER(uoffset, start, length, topdown);
#endif
	if (align != 0) {
		if ((*start & (align - 1)) != 0) {
			if (topdown)
				*start &= ~(align - 1);
			else
				*start = roundup(*start, align);
		}
		/*
		 * XXX Should we PMAP_PREFER() here again?
		 * eh...i think we're okay
		 */
	}

	/*
	 * Find the end of the proposed new region.  Be sure we didn't
	 * wrap around the address; if so, we lose.  Otherwise, if the
	 * proposed new region fits before the next entry, we win.
	 */

	end = *start + length;
	if (end < *start)
		return (-1);

	if (entry->next->start >= end && *start >= entry->end)
		return (1);

	return (0);
}

/*
 * uvm_map_findspace: find "length" sized space in "map".
 *
 * => "hint" is a hint about where we want it, unless UVM_FLAG_FIXED is
 *	set in "flags" (in which case we insist on using "hint").
 * => "result" is VA returned
 * => uobj/uoffset are to be used to handle VAC alignment, if required
 * => if "align" is non-zero, we attempt to align to that value.
 * => caller must at least have read-locked map
 * => returns NULL on failure, or pointer to prev. map entry if success
 * => note this is a cross between the old vm_map_findspace and vm_map_find
 */

struct vm_map_entry *
uvm_map_findspace(struct vm_map *map, vaddr_t hint, vsize_t length,
    vaddr_t *result /* OUT */, struct uvm_object *uobj, voff_t uoffset,
    vsize_t align, int flags)
{
	struct vm_map_entry *entry;
	struct vm_map_entry *child, *prev, *tmp;
	vaddr_t orig_hint;
	const int topdown = map->flags & VM_MAP_TOPDOWN;
	UVMHIST_FUNC("uvm_map_findspace");
	UVMHIST_CALLED(maphist);

	UVMHIST_LOG(maphist, "(map=0x%x, hint=0x%x, len=%d, flags=0x%x)",
	    map, hint, length, flags);
	KASSERT((align & (align - 1)) == 0);
	KASSERT((flags & UVM_FLAG_FIXED) == 0 || align == 0);

	uvm_tree_sanity(map, "map_findspace entry");

	/*
	 * remember the original hint.  if we are aligning, then we
	 * may have to try again with no alignment constraint if
	 * we fail the first time.
	 */

	orig_hint = hint;
	if (hint < vm_map_min(map)) {	/* check ranges ... */
		if (flags & UVM_FLAG_FIXED) {
			UVMHIST_LOG(maphist,"<- VA below map range",0,0,0,0);
			return (NULL);
		}
		hint = vm_map_min(map);
	}
	if (hint > vm_map_max(map)) {
		UVMHIST_LOG(maphist,"<- VA 0x%x > range [0x%x->0x%x]",
		    hint, vm_map_min(map), vm_map_max(map), 0);
		return (NULL);
	}

	/*
	 * Look for the first possible address; if there's already
	 * something at this address, we have to start after it.
	 */

	/*
	 * @@@: there are four, no, eight cases to consider.
	 *
	 * 0: found,     fixed,     bottom up -> fail
	 * 1: found,     fixed,     top down  -> fail
	 * 2: found,     not fixed, bottom up -> start after entry->end,
	 *                                       loop up
	 * 3: found,     not fixed, top down  -> start before entry->start,
	 *                                       loop down
	 * 4: not found, fixed,     bottom up -> check entry->next->start, fail
	 * 5: not found, fixed,     top down  -> check entry->next->start, fail
	 * 6: not found, not fixed, bottom up -> check entry->next->start,
	 *                                       loop up
	 * 7: not found, not fixed, top down  -> check entry->next->start,
	 *                                       loop down
	 *
	 * as you can see, it reduces to roughly five cases, and that
	 * adding top down mapping only adds one unique case (without
	 * it, there would be four cases).
	 */

	if ((flags & UVM_FLAG_FIXED) == 0 && hint == vm_map_min(map)) {
		entry = map->first_free;
	} else {
		if (uvm_map_lookup_entry(map, hint, &entry)) {
			/* "hint" address already in use ... */
			if (flags & UVM_FLAG_FIXED) {
				UVMHIST_LOG(maphist, "<- fixed & VA in use",
				    0, 0, 0, 0);
				return (NULL);
			}
			if (topdown)
				/* Start from lower gap. */
				entry = entry->prev;
		} else if (flags & UVM_FLAG_FIXED) {
			if (entry->next->start >= hint + length &&
			    hint + length > hint)
				goto found;

			/* "hint" address is gap but too small */
			UVMHIST_LOG(maphist, "<- fixed mapping failed",
			    0, 0, 0, 0);
			return (NULL); /* only one shot at it ... */
		} else {
			/*
			 * See if given hint fits in this gap.
			 */
			switch (uvm_map_space_avail(&hint, length,
			    uoffset, align, topdown, entry)) {
			case 1:
				goto found;
			case -1:
				goto wraparound;
			}

			if (topdown) {
				/*
				 * Still there is a chance to fit
				 * if hint > entry->end.
				 */
			} else {
				/* Start from higher gap. */
				entry = entry->next;
				if (entry == &map->header)
					goto notfound;
				goto nextgap;
			}
		}
	}

	/*
	 * Note that all UVM_FLAGS_FIXED case is already handled.
	 */
	KDASSERT((flags & UVM_FLAG_FIXED) == 0);

	/* Try to find the space in the red-black tree */

	/* Check slot before any entry */
	hint = topdown ? entry->next->start - length : entry->end;
	switch (uvm_map_space_avail(&hint, length, uoffset, align,
	    topdown, entry)) {
	case 1:
		goto found;
	case -1:
		goto wraparound;
	}

nextgap:
	KDASSERT((flags & UVM_FLAG_FIXED) == 0);
	/* If there is not enough space in the whole tree, we fail */
	tmp = RB_ROOT(&map->rbhead);
	if (tmp == NULL || tmp->space < length)
		goto notfound;

	prev = NULL; /* previous candidate */

	/* Find an entry close to hint that has enough space */
	for (; tmp;) {
		KASSERT(tmp->next->start == tmp->end + tmp->ownspace);
		if (topdown) {
			if (tmp->next->start < hint + length &&
			    (prev == NULL || tmp->end > prev->end)) {
				if (tmp->ownspace >= length)
					prev = tmp;
				else if ((child = RB_LEFT(tmp, rb_entry))
				    != NULL && child->space >= length)
					prev = tmp;
			}
		} else {
			if (tmp->end >= hint &&
			    (prev == NULL || tmp->end < prev->end)) {
				if (tmp->ownspace >= length)
					prev = tmp;
				else if ((child = RB_RIGHT(tmp, rb_entry))
				    != NULL && child->space >= length)
					prev = tmp;
			}
		}
		if (tmp->next->start < hint + length)
			child = RB_RIGHT(tmp, rb_entry);
		else if (tmp->end > hint)
			child = RB_LEFT(tmp, rb_entry);
		else {
			if (tmp->ownspace >= length)
				break;
			if (topdown)
				child = RB_LEFT(tmp, rb_entry);
			else
				child = RB_RIGHT(tmp, rb_entry);
		}
		if (child == NULL || child->space < length)
			break;
		tmp = child;
	}

	if (tmp != NULL && tmp->start < hint && hint < tmp->next->start) {
		/*
		 * Check if the entry that we found satifies the
		 * space requirement
		 */
		if (topdown) {
			if (hint > tmp->next->start - length)
				hint = tmp->next->start - length;
		} else {
			if (hint < tmp->end)
				hint = tmp->end;
		}
		switch (uvm_map_space_avail(&hint, length, uoffset, align,
		    topdown, tmp)) {
		case 1:
			entry = tmp;
			goto found;
		case -1:
			goto wraparound;
		}
		if (tmp->ownspace >= length)
			goto listsearch;
	}
	if (prev == NULL)
		goto notfound;

	if (topdown) {
		KASSERT(orig_hint >= prev->next->start - length ||
		    prev->next->start - length > prev->next->start);
		hint = prev->next->start - length;
	} else {
		KASSERT(orig_hint <= prev->end);
		hint = prev->end;
	}
	switch (uvm_map_space_avail(&hint, length, uoffset, align,
	    topdown, prev)) {
	case 1:
		entry = prev;
		goto found;
	case -1:
		goto wraparound;
	}
	if (prev->ownspace >= length)
		goto listsearch;

	if (topdown)
		tmp = RB_LEFT(prev, rb_entry);
	else
		tmp = RB_RIGHT(prev, rb_entry);
	for (;;) {
		KASSERT(tmp && tmp->space >= length);
		if (topdown)
			child = RB_RIGHT(tmp, rb_entry);
		else
			child = RB_LEFT(tmp, rb_entry);
		if (child && child->space >= length) {
			tmp = child;
			continue;
		}
		if (tmp->ownspace >= length)
			break;
		if (topdown)
			tmp = RB_LEFT(tmp, rb_entry);
		else
			tmp = RB_RIGHT(tmp, rb_entry);
	}

	if (topdown) {
		KASSERT(orig_hint >= tmp->next->start - length ||
		    tmp->next->start - length > tmp->next->start);
		hint = tmp->next->start - length;
	} else {
		KASSERT(orig_hint <= tmp->end);
		hint = tmp->end;
	}
	switch (uvm_map_space_avail(&hint, length, uoffset, align,
	    topdown, tmp)) {
	case 1:
		entry = tmp;
		goto found;
	case -1:
		goto wraparound;
	}

	/*
	 * The tree fails to find an entry because of offset or alignment
	 * restrictions.  Search the list instead.
	 */
 listsearch:
	/*
	 * Look through the rest of the map, trying to fit a new region in
	 * the gap between existing regions, or after the very last region.
	 * note: entry->end = base VA of current gap,
	 *	 entry->next->start = VA of end of current gap
	 */

	for (;;) {
		/* Update hint for current gap. */
		hint = topdown ? entry->next->start - length : entry->end;

		/* See if it fits. */
		switch (uvm_map_space_avail(&hint, length, uoffset, align,
		    topdown, entry)) {
		case 1:
			goto found;
		case -1:
			goto wraparound;
		}

		/* Advance to next/previous gap */
		if (topdown) {
			if (entry == &map->header) {
				UVMHIST_LOG(maphist, "<- failed (off start)",
				    0,0,0,0);
				goto notfound;
			}
			entry = entry->prev;
		} else {
			entry = entry->next;
			if (entry == &map->header) {
				UVMHIST_LOG(maphist, "<- failed (off end)",
				    0,0,0,0);
				goto notfound;
			}
		}
	}

 found:
	SAVE_HINT(map, map->hint, entry);
	*result = hint;
	UVMHIST_LOG(maphist,"<- got it!  (result=0x%x)", hint, 0,0,0);
	KASSERT( topdown || hint >= orig_hint);
	KASSERT(!topdown || hint <= orig_hint);
	KASSERT(entry->end <= hint);
	KASSERT(hint + length <= entry->next->start);
	return (entry);

 wraparound:
	UVMHIST_LOG(maphist, "<- failed (wrap around)", 0,0,0,0);

	return (NULL);

 notfound:
	UVMHIST_LOG(maphist, "<- failed (notfound)", 0,0,0,0);

	return (NULL);
}

/*
 *   U N M A P   -   m a i n   h e l p e r   f u n c t i o n s
 */

/*
 * uvm_unmap_remove: remove mappings from a vm_map (from "start" up to "stop")
 *
 * => caller must check alignment and size
 * => map must be locked by caller
 * => we return a list of map entries that we've remove from the map
 *    in "entry_list"
 */

void
uvm_unmap_remove(struct vm_map *map, vaddr_t start, vaddr_t end,
    struct vm_map_entry **entry_list /* OUT */,
    struct uvm_mapent_reservation *umr)
{
	struct vm_map_entry *entry, *first_entry, *next;
	vaddr_t len;
	UVMHIST_FUNC("uvm_unmap_remove"); UVMHIST_CALLED(maphist);

	UVMHIST_LOG(maphist,"(map=0x%x, start=0x%x, end=0x%x)",
	    map, start, end, 0);
	VM_MAP_RANGE_CHECK(map, start, end);

	uvm_tree_sanity(map, "unmap_remove entry");

	/*
	 * find first entry
	 */

	if (uvm_map_lookup_entry(map, start, &first_entry) == TRUE) {
		/* clip and go... */
		entry = first_entry;
		UVM_MAP_CLIP_START(map, entry, start, umr);
		/* critical!  prevents stale hint */
		SAVE_HINT(map, entry, entry->prev);
	} else {
		entry = first_entry->next;
	}

	/*
	 * Save the free space hint
	 */

	if (map->first_free->start >= start)
		map->first_free = entry->prev;

	/*
	 * note: we now re-use first_entry for a different task.  we remove
	 * a number of map entries from the map and save them in a linked
	 * list headed by "first_entry".  once we remove them from the map
	 * the caller should unlock the map and drop the references to the
	 * backing objects [c.f. uvm_unmap_detach].  the object is to
	 * separate unmapping from reference dropping.  why?
	 *   [1] the map has to be locked for unmapping
	 *   [2] the map need not be locked for reference dropping
	 *   [3] dropping references may trigger pager I/O, and if we hit
	 *       a pager that does synchronous I/O we may have to wait for it.
	 *   [4] we would like all waiting for I/O to occur with maps unlocked
	 *       so that we don't block other threads.
	 */

	first_entry = NULL;
	*entry_list = NULL;

	/*
	 * break up the area into map entry sized regions and unmap.  note
	 * that all mappings have to be removed before we can even consider
	 * dropping references to amaps or VM objects (otherwise we could end
	 * up with a mapping to a page on the free list which would be very bad)
	 */

	while ((entry != &map->header) && (entry->start < end)) {
		KASSERT((entry->flags & UVM_MAP_FIRST) == 0);

		UVM_MAP_CLIP_END(map, entry, end, umr);
		next = entry->next;
		len = entry->end - entry->start;

		/*
		 * unwire before removing addresses from the pmap; otherwise
		 * unwiring will put the entries back into the pmap (XXX).
		 */

		if (VM_MAPENT_ISWIRED(entry)) {
			uvm_map_entry_unwire(map, entry);
		}
		if ((map->flags & VM_MAP_PAGEABLE) == 0) {

			/*
			 * if the map is non-pageable, any pages mapped there
			 * must be wired and entered with pmap_kenter_pa(),
			 * and we should free any such pages immediately.
			 * this is mostly used for kmem_map and mb_map.
			 */

			if ((entry->flags & UVM_MAP_KMAPENT) == 0) {
				uvm_km_pgremove_intrsafe(entry->start,
				    entry->end);
				pmap_kremove(entry->start, len);
			}
		} else if (UVM_ET_ISOBJ(entry) &&
			   UVM_OBJ_IS_KERN_OBJECT(entry->object.uvm_obj)) {
			KASSERT(vm_map_pmap(map) == pmap_kernel());

			/*
			 * note: kernel object mappings are currently used in
			 * two ways:
			 *  [1] "normal" mappings of pages in the kernel object
			 *  [2] uvm_km_valloc'd allocations in which we
			 *      pmap_enter in some non-kernel-object page
			 *      (e.g. vmapbuf).
			 *
			 * for case [1], we need to remove the mapping from
			 * the pmap and then remove the page from the kernel
			 * object (because, once pages in a kernel object are
			 * unmapped they are no longer needed, unlike, say,
			 * a vnode where you might want the data to persist
			 * until flushed out of a queue).
			 *
			 * for case [2], we need to remove the mapping from
			 * the pmap.  there shouldn't be any pages at the
			 * specified offset in the kernel object [but it
			 * doesn't hurt to call uvm_km_pgremove just to be
			 * safe?]
			 *
			 * uvm_km_pgremove currently does the following:
			 *   for pages in the kernel object in range:
			 *     - drops the swap slot
			 *     - uvm_pagefree the page
			 */

			/*
			 * remove mappings from pmap and drop the pages
			 * from the object.  offsets are always relative
			 * to vm_map_min(kernel_map).
			 */

			pmap_remove(pmap_kernel(), entry->start,
			    entry->start + len);
			uvm_km_pgremove(entry->object.uvm_obj,
			    entry->start - vm_map_min(kernel_map),
			    entry->end - vm_map_min(kernel_map));

			/*
			 * null out kernel_object reference, we've just
			 * dropped it
			 */

			entry->etype &= ~UVM_ET_OBJ;
			entry->object.uvm_obj = NULL;
		} else if (UVM_ET_ISOBJ(entry) || entry->aref.ar_amap) {

			/*
			 * remove mappings the standard way.
			 */

			pmap_remove(map->pmap, entry->start, entry->end);
		}

#if defined(DEBUG)
		if ((entry->flags & UVM_MAP_KMAPENT) == 0) {

			/*
			 * check if there's remaining mapping,
			 * which is a bug in caller.
			 */

			vaddr_t va;
			for (va = entry->start; va < entry->end;
			    va += PAGE_SIZE) {
				if (pmap_extract(vm_map_pmap(map), va, NULL)) {
					panic("uvm_unmap_remove: has mapping");
				}
			}
		}
#endif /* defined(DEBUG) */

		/*
		 * remove entry from map and put it on our list of entries
		 * that we've nuked.  then go to next entry.
		 */

		UVMHIST_LOG(maphist, "  removed map entry 0x%x", entry, 0, 0,0);

		/* critical!  prevents stale hint */
		SAVE_HINT(map, entry, entry->prev);

		uvm_map_entry_unlink(map, entry);
		KASSERT(map->size >= len);
		map->size -= len;
		entry->prev = NULL;
		entry->next = first_entry;
		first_entry = entry;
		entry = next;
	}
	if ((map->flags & VM_MAP_DYING) == 0) {
		pmap_update(vm_map_pmap(map));
	}

	uvm_tree_sanity(map, "unmap_remove leave");

	/*
	 * now we've cleaned up the map and are ready for the caller to drop
	 * references to the mapped objects.
	 */

	*entry_list = first_entry;
	UVMHIST_LOG(maphist,"<- done!", 0, 0, 0, 0);

	simple_lock(&map->flags_lock);
	if (map->flags & VM_MAP_WANTVA) {
		map->flags &= ~VM_MAP_WANTVA;
		wakeup(&map->header);
	}
	simple_unlock(&map->flags_lock);
}

/*
 * uvm_unmap_detach: drop references in a chain of map entries
 *
 * => we will free the map entries as we traverse the list.
 */

void
uvm_unmap_detach(struct vm_map_entry *first_entry, int flags)
{
	struct vm_map_entry *next_entry;
	UVMHIST_FUNC("uvm_unmap_detach"); UVMHIST_CALLED(maphist);

	while (first_entry) {
		KASSERT(!VM_MAPENT_ISWIRED(first_entry));
		UVMHIST_LOG(maphist,
		    "  detach 0x%x: amap=0x%x, obj=0x%x, submap?=%d",
		    first_entry, first_entry->aref.ar_amap,
		    first_entry->object.uvm_obj,
		    UVM_ET_ISSUBMAP(first_entry));

		/*
		 * drop reference to amap, if we've got one
		 */

		if (first_entry->aref.ar_amap)
			uvm_map_unreference_amap(first_entry, flags);

		/*
		 * drop reference to our backing object, if we've got one
		 */

		KASSERT(!UVM_ET_ISSUBMAP(first_entry));
		if (UVM_ET_ISOBJ(first_entry) &&
		    first_entry->object.uvm_obj->pgops->pgo_detach) {
			(*first_entry->object.uvm_obj->pgops->pgo_detach)
				(first_entry->object.uvm_obj);
		}
		next_entry = first_entry->next;
		uvm_mapent_free(first_entry);
		first_entry = next_entry;
	}
	UVMHIST_LOG(maphist, "<- done", 0,0,0,0);
}

/*
 *   E X T R A C T I O N   F U N C T I O N S
 */

/*
 * uvm_map_reserve: reserve space in a vm_map for future use.
 *
 * => we reserve space in a map by putting a dummy map entry in the
 *    map (dummy means obj=NULL, amap=NULL, prot=VM_PROT_NONE)
 * => map should be unlocked (we will write lock it)
 * => we return true if we were able to reserve space
 * => XXXCDC: should be inline?
 */

int
uvm_map_reserve(struct vm_map *map, vsize_t size,
    vaddr_t offset	/* hint for pmap_prefer */,
    vsize_t align	/* alignment hint */,
    vaddr_t *raddr	/* IN:hint, OUT: reserved VA */)
{
	UVMHIST_FUNC("uvm_map_reserve"); UVMHIST_CALLED(maphist);

	UVMHIST_LOG(maphist, "(map=0x%x, size=0x%x, offset=0x%x,addr=0x%x)",
	    map,size,offset,raddr);

	size = round_page(size);
	if (*raddr < vm_map_min(map))
		*raddr = vm_map_min(map);		/* hint */

	/*
	 * reserve some virtual space.
	 */

	if (uvm_map(map, raddr, size, NULL, offset, 0,
	    UVM_MAPFLAG(UVM_PROT_NONE, UVM_PROT_NONE, UVM_INH_NONE,
	    UVM_ADV_RANDOM, UVM_FLAG_NOMERGE)) != 0) {
	    UVMHIST_LOG(maphist, "<- done (no VM)", 0,0,0,0);
		return (FALSE);
	}

	UVMHIST_LOG(maphist, "<- done (*raddr=0x%x)", *raddr,0,0,0);
	return (TRUE);
}

/*
 * uvm_map_replace: replace a reserved (blank) area of memory with
 * real mappings.
 *
 * => caller must WRITE-LOCK the map
 * => we return TRUE if replacement was a success
 * => we expect the newents chain to have nnewents entrys on it and
 *    we expect newents->prev to point to the last entry on the list
 * => note newents is allowed to be NULL
 */

int
uvm_map_replace(struct vm_map *map, vaddr_t start, vaddr_t end,
    struct vm_map_entry *newents, int nnewents)
{
	struct vm_map_entry *oldent, *last;

	uvm_tree_sanity(map, "map_replace entry");

	/*
	 * first find the blank map entry at the specified address
	 */

	if (!uvm_map_lookup_entry(map, start, &oldent)) {
		return (FALSE);
	}

	/*
	 * check to make sure we have a proper blank entry
	 */

	if (oldent->start != start || oldent->end != end ||
	    oldent->object.uvm_obj != NULL || oldent->aref.ar_amap != NULL) {
		return (FALSE);
	}

#ifdef DIAGNOSTIC

	/*
	 * sanity check the newents chain
	 */

	{
		struct vm_map_entry *tmpent = newents;
		int nent = 0;
		vaddr_t cur = start;

		while (tmpent) {
			nent++;
			if (tmpent->start < cur)
				panic("uvm_map_replace1");
			if (tmpent->start > tmpent->end || tmpent->end > end) {
		printf("tmpent->start=0x%lx, tmpent->end=0x%lx, end=0x%lx\n",
			    tmpent->start, tmpent->end, end);
				panic("uvm_map_replace2");
			}
			cur = tmpent->end;
			if (tmpent->next) {
				if (tmpent->next->prev != tmpent)
					panic("uvm_map_replace3");
			} else {
				if (newents->prev != tmpent)
					panic("uvm_map_replace4");
			}
			tmpent = tmpent->next;
		}
		if (nent != nnewents)
			panic("uvm_map_replace5");
	}
#endif

	/*
	 * map entry is a valid blank!   replace it.   (this does all the
	 * work of map entry link/unlink...).
	 */

	if (newents) {
		last = newents->prev;

		/* critical: flush stale hints out of map */
		SAVE_HINT(map, map->hint, newents);
		if (map->first_free == oldent)
			map->first_free = last;

		last->next = oldent->next;
		last->next->prev = last;

		/* Fix RB tree */
		uvm_rb_remove(map, oldent);

		newents->prev = oldent->prev;
		newents->prev->next = newents;
		map->nentries = map->nentries + (nnewents - 1);

		/* Fixup the RB tree */
		{
			int i;
			struct vm_map_entry *tmp;

			tmp = newents;
			for (i = 0; i < nnewents && tmp; i++) {
				uvm_rb_insert(map, tmp);
				tmp = tmp->next;
			}
		}
	} else {

		/* critical: flush stale hints out of map */
		SAVE_HINT(map, map->hint, oldent->prev);
		if (map->first_free == oldent)
			map->first_free = oldent->prev;

		/* NULL list of new entries: just remove the old one */
		uvm_map_entry_unlink(map, oldent);
	}

	uvm_tree_sanity(map, "map_replace leave");

	/*
	 * now we can free the old blank entry, unlock the map and return.
	 */

	uvm_mapent_free(oldent);
	return (TRUE);
}

/*
 * uvm_map_extract: extract a mapping from a map and put it somewhere
 *	(maybe removing the old mapping)
 *
 * => maps should be unlocked (we will write lock them)
 * => returns 0 on success, error code otherwise
 * => start must be page aligned
 * => len must be page sized
 * => flags:
 *      UVM_EXTRACT_REMOVE: remove mappings from srcmap
 *      UVM_EXTRACT_CONTIG: abort if unmapped area (advisory only)
 *      UVM_EXTRACT_QREF: for a temporary extraction do quick obj refs
 *      UVM_EXTRACT_FIXPROT: set prot to maxprot as we go
 *    >>>NOTE: if you set REMOVE, you are not allowed to use CONTIG or QREF!<<<
 *    >>>NOTE: QREF's must be unmapped via the QREF path, thus should only
 *             be used from within the kernel in a kernel level map <<<
 */

int
uvm_map_extract(struct vm_map *srcmap, vaddr_t start, vsize_t len,
    struct vm_map *dstmap, vaddr_t *dstaddrp, int flags)
{
	vaddr_t dstaddr, end, newend, oldoffset, fudge, orig_fudge;
	struct vm_map_entry *chain, *endchain, *entry, *orig_entry, *newentry,
	    *deadentry, *oldentry;
	vsize_t elen;
	int nchain, error, copy_ok;
	UVMHIST_FUNC("uvm_map_extract"); UVMHIST_CALLED(maphist);

	UVMHIST_LOG(maphist,"(srcmap=0x%x,start=0x%x, len=0x%x", srcmap, start,
	    len,0);
	UVMHIST_LOG(maphist," ...,dstmap=0x%x, flags=0x%x)", dstmap,flags,0,0);

	uvm_tree_sanity(srcmap, "map_extract src enter");
	uvm_tree_sanity(dstmap, "map_extract dst enter");

	/*
	 * step 0: sanity check: start must be on a page boundary, length
	 * must be page sized.  can't ask for CONTIG/QREF if you asked for
	 * REMOVE.
	 */

	KASSERT((start & PAGE_MASK) == 0 && (len & PAGE_MASK) == 0);
	KASSERT((flags & UVM_EXTRACT_REMOVE) == 0 ||
		(flags & (UVM_EXTRACT_CONTIG|UVM_EXTRACT_QREF)) == 0);

	/*
	 * step 1: reserve space in the target map for the extracted area
	 */

	dstaddr = vm_map_min(dstmap);
	if (uvm_map_reserve(dstmap, len, start, 0, &dstaddr) == FALSE)
		return (ENOMEM);
	*dstaddrp = dstaddr;	/* pass address back to caller */
	UVMHIST_LOG(maphist, "  dstaddr=0x%x", dstaddr,0,0,0);

	/*
	 * step 2: setup for the extraction process loop by init'ing the
	 * map entry chain, locking src map, and looking up the first useful
	 * entry in the map.
	 */

	end = start + len;
	newend = dstaddr + len;
	chain = endchain = NULL;
	nchain = 0;
	vm_map_lock(srcmap);

	if (uvm_map_lookup_entry(srcmap, start, &entry)) {

		/* "start" is within an entry */
		if (flags & UVM_EXTRACT_QREF) {

			/*
			 * for quick references we don't clip the entry, so
			 * the entry may map space "before" the starting
			 * virtual address... this is the "fudge" factor
			 * (which can be non-zero only the first time
			 * through the "while" loop in step 3).
			 */

			fudge = start - entry->start;
		} else {

			/*
			 * normal reference: we clip the map to fit (thus
			 * fudge is zero)
			 */

			UVM_MAP_CLIP_START(srcmap, entry, start, NULL);
			SAVE_HINT(srcmap, srcmap->hint, entry->prev);
			fudge = 0;
		}
	} else {

		/* "start" is not within an entry ... skip to next entry */
		if (flags & UVM_EXTRACT_CONTIG) {
			error = EINVAL;
			goto bad;    /* definite hole here ... */
		}

		entry = entry->next;
		fudge = 0;
	}

	/* save values from srcmap for step 6 */
	orig_entry = entry;
	orig_fudge = fudge;

	/*
	 * step 3: now start looping through the map entries, extracting
	 * as we go.
	 */

	while (entry->start < end && entry != &srcmap->header) {

		/* if we are not doing a quick reference, clip it */
		if ((flags & UVM_EXTRACT_QREF) == 0)
			UVM_MAP_CLIP_END(srcmap, entry, end, NULL);

		/* clear needs_copy (allow chunking) */
		if (UVM_ET_ISNEEDSCOPY(entry)) {
			amap_copy(srcmap, entry, M_NOWAIT, TRUE, start, end);
			if (UVM_ET_ISNEEDSCOPY(entry)) {  /* failed? */
				error = ENOMEM;
				goto bad;
			}

			/* amap_copy could clip (during chunk)!  update fudge */
			if (fudge) {
				fudge = start - entry->start;
				orig_fudge = fudge;
			}
		}

		/* calculate the offset of this from "start" */
		oldoffset = (entry->start + fudge) - start;

		/* allocate a new map entry */
		newentry = uvm_mapent_alloc(dstmap, 0);
		if (newentry == NULL) {
			error = ENOMEM;
			goto bad;
		}

		/* set up new map entry */
		newentry->next = NULL;
		newentry->prev = endchain;
		newentry->start = dstaddr + oldoffset;
		newentry->end =
		    newentry->start + (entry->end - (entry->start + fudge));
		if (newentry->end > newend || newentry->end < newentry->start)
			newentry->end = newend;
		newentry->object.uvm_obj = entry->object.uvm_obj;
		if (newentry->object.uvm_obj) {
			if (newentry->object.uvm_obj->pgops->pgo_reference)
				newentry->object.uvm_obj->pgops->
				    pgo_reference(newentry->object.uvm_obj);
				newentry->offset = entry->offset + fudge;
		} else {
			newentry->offset = 0;
		}
		newentry->etype = entry->etype;
		newentry->protection = (flags & UVM_EXTRACT_FIXPROT) ?
			entry->max_protection : entry->protection;
		newentry->max_protection = entry->max_protection;
		newentry->inheritance = entry->inheritance;
		newentry->wired_count = 0;
		newentry->aref.ar_amap = entry->aref.ar_amap;
		if (newentry->aref.ar_amap) {
			newentry->aref.ar_pageoff =
			    entry->aref.ar_pageoff + (fudge >> PAGE_SHIFT);
			uvm_map_reference_amap(newentry, AMAP_SHARED |
			    ((flags & UVM_EXTRACT_QREF) ? AMAP_REFALL : 0));
		} else {
			newentry->aref.ar_pageoff = 0;
		}
		newentry->advice = entry->advice;

		/* now link it on the chain */
		nchain++;
		if (endchain == NULL) {
			chain = endchain = newentry;
		} else {
			endchain->next = newentry;
			endchain = newentry;
		}

		/* end of 'while' loop! */
		if ((flags & UVM_EXTRACT_CONTIG) && entry->end < end &&
		    (entry->next == &srcmap->header ||
		    entry->next->start != entry->end)) {
			error = EINVAL;
			goto bad;
		}
		entry = entry->next;
		fudge = 0;
	}

	/*
	 * step 4: close off chain (in format expected by uvm_map_replace)
	 */

	if (chain)
		chain->prev = endchain;

	/*
	 * step 5: attempt to lock the dest map so we can pmap_copy.
	 * note usage of copy_ok:
	 *   1 => dstmap locked, pmap_copy ok, and we "replace" here (step 5)
	 *   0 => dstmap unlocked, NO pmap_copy, and we will "replace" in step 7
	 */

	if (srcmap == dstmap || vm_map_lock_try(dstmap) == TRUE) {
		copy_ok = 1;
		if (!uvm_map_replace(dstmap, dstaddr, dstaddr+len, chain,
		    nchain)) {
			if (srcmap != dstmap)
				vm_map_unlock(dstmap);
			error = EIO;
			goto bad;
		}
	} else {
		copy_ok = 0;
		/* replace defered until step 7 */
	}

	/*
	 * step 6: traverse the srcmap a second time to do the following:
	 *  - if we got a lock on the dstmap do pmap_copy
	 *  - if UVM_EXTRACT_REMOVE remove the entries
	 * we make use of orig_entry and orig_fudge (saved in step 2)
	 */

	if (copy_ok || (flags & UVM_EXTRACT_REMOVE)) {

		/* purge possible stale hints from srcmap */
		if (flags & UVM_EXTRACT_REMOVE) {
			SAVE_HINT(srcmap, srcmap->hint, orig_entry->prev);
			if (srcmap->first_free->start >= start)
				srcmap->first_free = orig_entry->prev;
		}

		entry = orig_entry;
		fudge = orig_fudge;
		deadentry = NULL;	/* for UVM_EXTRACT_REMOVE */

		while (entry->start < end && entry != &srcmap->header) {
			if (copy_ok) {
				oldoffset = (entry->start + fudge) - start;
				elen = MIN(end, entry->end) -
				    (entry->start + fudge);
				pmap_copy(dstmap->pmap, srcmap->pmap,
				    dstaddr + oldoffset, elen,
				    entry->start + fudge);
			}

			/* we advance "entry" in the following if statement */
			if (flags & UVM_EXTRACT_REMOVE) {
				pmap_remove(srcmap->pmap, entry->start,
						entry->end);
				oldentry = entry;	/* save entry */
				entry = entry->next;	/* advance */
				uvm_map_entry_unlink(srcmap, oldentry);
							/* add to dead list */
				oldentry->next = deadentry;
				deadentry = oldentry;
			} else {
				entry = entry->next;		/* advance */
			}

			/* end of 'while' loop */
			fudge = 0;
		}
		pmap_update(srcmap->pmap);

		/*
		 * unlock dstmap.  we will dispose of deadentry in
		 * step 7 if needed
		 */

		if (copy_ok && srcmap != dstmap)
			vm_map_unlock(dstmap);

	} else {
		deadentry = NULL;
	}

	/*
	 * step 7: we are done with the source map, unlock.   if copy_ok
	 * is 0 then we have not replaced the dummy mapping in dstmap yet
	 * and we need to do so now.
	 */

	vm_map_unlock(srcmap);
	if ((flags & UVM_EXTRACT_REMOVE) && deadentry)
		uvm_unmap_detach(deadentry, 0);   /* dispose of old entries */

	/* now do the replacement if we didn't do it in step 5 */
	if (copy_ok == 0) {
		vm_map_lock(dstmap);
		error = uvm_map_replace(dstmap, dstaddr, dstaddr+len, chain,
		    nchain);
		vm_map_unlock(dstmap);

		if (error == FALSE) {
			error = EIO;
			goto bad2;
		}
	}

	uvm_tree_sanity(srcmap, "map_extract src leave");
	uvm_tree_sanity(dstmap, "map_extract dst leave");

	return (0);

	/*
	 * bad: failure recovery
	 */
bad:
	vm_map_unlock(srcmap);
bad2:			/* src already unlocked */
	if (chain)
		uvm_unmap_detach(chain,
		    (flags & UVM_EXTRACT_QREF) ? AMAP_REFALL : 0);

	uvm_tree_sanity(srcmap, "map_extract src err leave");
	uvm_tree_sanity(dstmap, "map_extract dst err leave");

	uvm_unmap(dstmap, dstaddr, dstaddr+len);   /* ??? */
	return (error);
}

/* end of extraction functions */

/*
 * uvm_map_submap: punch down part of a map into a submap
 *
 * => only the kernel_map is allowed to be submapped
 * => the purpose of submapping is to break up the locking granularity
 *	of a larger map
 * => the range specified must have been mapped previously with a uvm_map()
 *	call [with uobj==NULL] to create a blank map entry in the main map.
 *	[And it had better still be blank!]
 * => maps which contain submaps should never be copied or forked.
 * => to remove a submap, use uvm_unmap() on the main map
 *	and then uvm_map_deallocate() the submap.
 * => main map must be unlocked.
 * => submap must have been init'd and have a zero reference count.
 *	[need not be locked as we don't actually reference it]
 */

int
uvm_map_submap(struct vm_map *map, vaddr_t start, vaddr_t end,
    struct vm_map *submap)
{
	struct vm_map_entry *entry;
	struct uvm_mapent_reservation umr;
	int error;

	uvm_mapent_reserve(map, &umr, 2, 0);

	vm_map_lock(map);
	VM_MAP_RANGE_CHECK(map, start, end);

	if (uvm_map_lookup_entry(map, start, &entry)) {
		UVM_MAP_CLIP_START(map, entry, start, &umr);
		UVM_MAP_CLIP_END(map, entry, end, &umr);	/* to be safe */
	} else {
		entry = NULL;
	}

	if (entry != NULL &&
	    entry->start == start && entry->end == end &&
	    entry->object.uvm_obj == NULL && entry->aref.ar_amap == NULL &&
	    !UVM_ET_ISCOPYONWRITE(entry) && !UVM_ET_ISNEEDSCOPY(entry)) {
		entry->etype |= UVM_ET_SUBMAP;
		entry->object.sub_map = submap;
		entry->offset = 0;
		uvm_map_reference(submap);
		error = 0;
	} else {
		error = EINVAL;
	}
	vm_map_unlock(map);

	uvm_mapent_unreserve(map, &umr);

	return error;
}

/*
 * uvm_map_setup_kernel: init in-kernel map
 *
 * => map must not be in service yet.
 */

void
uvm_map_setup_kernel(struct vm_map_kernel *map,
    vaddr_t min, vaddr_t max, int flags)
{

	uvm_map_setup(&map->vmk_map, min, max, flags);

	LIST_INIT(&map->vmk_kentry_free);
	map->vmk_merged_entries = NULL;
}


/*
 * uvm_map_protect: change map protection
 *
 * => set_max means set max_protection.
 * => map must be unlocked.
 */

#define MASK(entry)	(UVM_ET_ISCOPYONWRITE(entry) ? \
			 ~VM_PROT_WRITE : VM_PROT_ALL)

int
uvm_map_protect(struct vm_map *map, vaddr_t start, vaddr_t end,
    vm_prot_t new_prot, boolean_t set_max)
{
	struct vm_map_entry *current, *entry;
	int error = 0;
	UVMHIST_FUNC("uvm_map_protect"); UVMHIST_CALLED(maphist);
	UVMHIST_LOG(maphist,"(map=0x%x,start=0x%x,end=0x%x,new_prot=0x%x)",
		    map, start, end, new_prot);

	vm_map_lock(map);
	VM_MAP_RANGE_CHECK(map, start, end);
	if (uvm_map_lookup_entry(map, start, &entry)) {
		UVM_MAP_CLIP_START(map, entry, start, NULL);
	} else {
		entry = entry->next;
	}

	/*
	 * make a first pass to check for protection violations.
	 */

	current = entry;
	while ((current != &map->header) && (current->start < end)) {
		if (UVM_ET_ISSUBMAP(current)) {
			error = EINVAL;
			goto out;
		}
		if ((new_prot & current->max_protection) != new_prot) {
			error = EACCES;
			goto out;
		}
		/*
		 * Don't allow VM_PROT_EXECUTE to be set on entries that
		 * point to vnodes that are associated with a NOEXEC file
		 * system.
		 */
		if (UVM_ET_ISOBJ(current) &&
		    UVM_OBJ_IS_VNODE(current->object.uvm_obj)) {
			struct vnode *vp =
			    (struct vnode *) current->object.uvm_obj;

			if ((new_prot & VM_PROT_EXECUTE) != 0 &&
			    (vp->v_mount->mnt_flag & MNT_NOEXEC) != 0) {
				error = EACCES;
				goto out;
			}
		}
		current = current->next;
	}

	/* go back and fix up protections (no need to clip this time). */

	current = entry;
	while ((current != &map->header) && (current->start < end)) {
		vm_prot_t old_prot;

		UVM_MAP_CLIP_END(map, current, end, NULL);
		old_prot = current->protection;
		if (set_max)
			current->protection =
			    (current->max_protection = new_prot) & old_prot;
		else
			current->protection = new_prot;

		/*
		 * update physical map if necessary.  worry about copy-on-write
		 * here -- CHECK THIS XXX
		 */

		if (current->protection != old_prot) {
			/* update pmap! */
			pmap_protect(map->pmap, current->start, current->end,
			    current->protection & MASK(entry));

			/*
			 * If this entry points at a vnode, and the
			 * protection includes VM_PROT_EXECUTE, mark
			 * the vnode as VEXECMAP.
			 */
			if (UVM_ET_ISOBJ(current)) {
				struct uvm_object *uobj =
				    current->object.uvm_obj;

				if (UVM_OBJ_IS_VNODE(uobj) &&
				    (current->protection & VM_PROT_EXECUTE))
					vn_markexec((struct vnode *) uobj);
			}
		}

		/*
		 * If the map is configured to lock any future mappings,
		 * wire this entry now if the old protection was VM_PROT_NONE
		 * and the new protection is not VM_PROT_NONE.
		 */

		if ((map->flags & VM_MAP_WIREFUTURE) != 0 &&
		    VM_MAPENT_ISWIRED(entry) == 0 &&
		    old_prot == VM_PROT_NONE &&
		    new_prot != VM_PROT_NONE) {
			if (uvm_map_pageable(map, entry->start,
			    entry->end, FALSE,
			    UVM_LK_ENTER|UVM_LK_EXIT) != 0) {

				/*
				 * If locking the entry fails, remember the
				 * error if it's the first one.  Note we
				 * still continue setting the protection in
				 * the map, but will return the error
				 * condition regardless.
				 *
				 * XXX Ignore what the actual error is,
				 * XXX just call it a resource shortage
				 * XXX so that it doesn't get confused
				 * XXX what uvm_map_protect() itself would
				 * XXX normally return.
				 */

				error = ENOMEM;
			}
		}
		current = current->next;
	}
	pmap_update(map->pmap);

 out:
	vm_map_unlock(map);

	UVMHIST_LOG(maphist, "<- done, error=%d",error,0,0,0);
	return error;
}

#undef  MASK

/*
 * uvm_map_inherit: set inheritance code for range of addrs in map.
 *
 * => map must be unlocked
 * => note that the inherit code is used during a "fork".  see fork
 *	code for details.
 */

int
uvm_map_inherit(struct vm_map *map, vaddr_t start, vaddr_t end,
    vm_inherit_t new_inheritance)
{
	struct vm_map_entry *entry, *temp_entry;
	UVMHIST_FUNC("uvm_map_inherit"); UVMHIST_CALLED(maphist);
	UVMHIST_LOG(maphist,"(map=0x%x,start=0x%x,end=0x%x,new_inh=0x%x)",
	    map, start, end, new_inheritance);

	switch (new_inheritance) {
	case MAP_INHERIT_NONE:
	case MAP_INHERIT_COPY:
	case MAP_INHERIT_SHARE:
		break;
	default:
		UVMHIST_LOG(maphist,"<- done (INVALID ARG)",0,0,0,0);
		return EINVAL;
	}

	vm_map_lock(map);
	VM_MAP_RANGE_CHECK(map, start, end);
	if (uvm_map_lookup_entry(map, start, &temp_entry)) {
		entry = temp_entry;
		UVM_MAP_CLIP_START(map, entry, start, NULL);
	}  else {
		entry = temp_entry->next;
	}
	while ((entry != &map->header) && (entry->start < end)) {
		UVM_MAP_CLIP_END(map, entry, end, NULL);
		entry->inheritance = new_inheritance;
		entry = entry->next;
	}
	vm_map_unlock(map);
	UVMHIST_LOG(maphist,"<- done (OK)",0,0,0,0);
	return 0;
}

/*
 * uvm_map_advice: set advice code for range of addrs in map.
 *
 * => map must be unlocked
 */

int
uvm_map_advice(struct vm_map *map, vaddr_t start, vaddr_t end, int new_advice)
{
	struct vm_map_entry *entry, *temp_entry;
	UVMHIST_FUNC("uvm_map_advice"); UVMHIST_CALLED(maphist);
	UVMHIST_LOG(maphist,"(map=0x%x,start=0x%x,end=0x%x,new_adv=0x%x)",
	    map, start, end, new_advice);

	vm_map_lock(map);
	VM_MAP_RANGE_CHECK(map, start, end);
	if (uvm_map_lookup_entry(map, start, &temp_entry)) {
		entry = temp_entry;
		UVM_MAP_CLIP_START(map, entry, start, NULL);
	} else {
		entry = temp_entry->next;
	}

	/*
	 * XXXJRT: disallow holes?
	 */

	while ((entry != &map->header) && (entry->start < end)) {
		UVM_MAP_CLIP_END(map, entry, end, NULL);

		switch (new_advice) {
		case MADV_NORMAL:
		case MADV_RANDOM:
		case MADV_SEQUENTIAL:
			/* nothing special here */
			break;

		default:
			vm_map_unlock(map);
			UVMHIST_LOG(maphist,"<- done (INVALID ARG)",0,0,0,0);
			return EINVAL;
		}
		entry->advice = new_advice;
		entry = entry->next;
	}

	vm_map_unlock(map);
	UVMHIST_LOG(maphist,"<- done (OK)",0,0,0,0);
	return 0;
}

/*
 * uvm_map_pageable: sets the pageability of a range in a map.
 *
 * => wires map entries.  should not be used for transient page locking.
 *	for that, use uvm_fault_wire()/uvm_fault_unwire() (see uvm_vslock()).
 * => regions sepcified as not pageable require lock-down (wired) memory
 *	and page tables.
 * => map must never be read-locked
 * => if islocked is TRUE, map is already write-locked
 * => we always unlock the map, since we must downgrade to a read-lock
 *	to call uvm_fault_wire()
 * => XXXCDC: check this and try and clean it up.
 */

int
uvm_map_pageable(struct vm_map *map, vaddr_t start, vaddr_t end,
    boolean_t new_pageable, int lockflags)
{
	struct vm_map_entry *entry, *start_entry, *failed_entry;
	int rv;
#ifdef DIAGNOSTIC
	u_int timestamp_save;
#endif
	UVMHIST_FUNC("uvm_map_pageable"); UVMHIST_CALLED(maphist);
	UVMHIST_LOG(maphist,"(map=0x%x,start=0x%x,end=0x%x,new_pageable=0x%x)",
		    map, start, end, new_pageable);
	KASSERT(map->flags & VM_MAP_PAGEABLE);

	if ((lockflags & UVM_LK_ENTER) == 0)
		vm_map_lock(map);
	VM_MAP_RANGE_CHECK(map, start, end);

	/*
	 * only one pageability change may take place at one time, since
	 * uvm_fault_wire assumes it will be called only once for each
	 * wiring/unwiring.  therefore, we have to make sure we're actually
	 * changing the pageability for the entire region.  we do so before
	 * making any changes.
	 */

	if (uvm_map_lookup_entry(map, start, &start_entry) == FALSE) {
		if ((lockflags & UVM_LK_EXIT) == 0)
			vm_map_unlock(map);

		UVMHIST_LOG(maphist,"<- done (fault)",0,0,0,0);
		return EFAULT;
	}
	entry = start_entry;

	/*
	 * handle wiring and unwiring separately.
	 */

	if (new_pageable) {		/* unwire */
		UVM_MAP_CLIP_START(map, entry, start, NULL);

		/*
		 * unwiring.  first ensure that the range to be unwired is
		 * really wired down and that there are no holes.
		 */

		while ((entry != &map->header) && (entry->start < end)) {
			if (entry->wired_count == 0 ||
			    (entry->end < end &&
			     (entry->next == &map->header ||
			      entry->next->start > entry->end))) {
				if ((lockflags & UVM_LK_EXIT) == 0)
					vm_map_unlock(map);
				UVMHIST_LOG(maphist, "<- done (INVAL)",0,0,0,0);
				return EINVAL;
			}
			entry = entry->next;
		}

		/*
		 * POSIX 1003.1b - a single munlock call unlocks a region,
		 * regardless of the number of mlock calls made on that
		 * region.
		 */

		entry = start_entry;
		while ((entry != &map->header) && (entry->start < end)) {
			UVM_MAP_CLIP_END(map, entry, end, NULL);
			if (VM_MAPENT_ISWIRED(entry))
				uvm_map_entry_unwire(map, entry);
			entry = entry->next;
		}
		if ((lockflags & UVM_LK_EXIT) == 0)
			vm_map_unlock(map);
		UVMHIST_LOG(maphist,"<- done (OK UNWIRE)",0,0,0,0);
		return 0;
	}

	/*
	 * wire case: in two passes [XXXCDC: ugly block of code here]
	 *
	 * 1: holding the write lock, we create any anonymous maps that need
	 *    to be created.  then we clip each map entry to the region to
	 *    be wired and increment its wiring count.
	 *
	 * 2: we downgrade to a read lock, and call uvm_fault_wire to fault
	 *    in the pages for any newly wired area (wired_count == 1).
	 *
	 *    downgrading to a read lock for uvm_fault_wire avoids a possible
	 *    deadlock with another thread that may have faulted on one of
	 *    the pages to be wired (it would mark the page busy, blocking
	 *    us, then in turn block on the map lock that we hold).  because
	 *    of problems in the recursive lock package, we cannot upgrade
	 *    to a write lock in vm_map_lookup.  thus, any actions that
	 *    require the write lock must be done beforehand.  because we
	 *    keep the read lock on the map, the copy-on-write status of the
	 *    entries we modify here cannot change.
	 */

	while ((entry != &map->header) && (entry->start < end)) {
		if (VM_MAPENT_ISWIRED(entry) == 0) { /* not already wired? */

			/*
			 * perform actions of vm_map_lookup that need the
			 * write lock on the map: create an anonymous map
			 * for a copy-on-write region, or an anonymous map
			 * for a zero-fill region.  (XXXCDC: submap case
			 * ok?)
			 */

			if (!UVM_ET_ISSUBMAP(entry)) {  /* not submap */
				if (UVM_ET_ISNEEDSCOPY(entry) &&
				    ((entry->max_protection & VM_PROT_WRITE) ||
				     (entry->object.uvm_obj == NULL))) {
					amap_copy(map, entry, M_WAITOK, TRUE,
					    start, end);
					/* XXXCDC: wait OK? */
				}
			}
		}
		UVM_MAP_CLIP_START(map, entry, start, NULL);
		UVM_MAP_CLIP_END(map, entry, end, NULL);
		entry->wired_count++;

		/*
		 * Check for holes
		 */

		if (entry->protection == VM_PROT_NONE ||
		    (entry->end < end &&
		     (entry->next == &map->header ||
		      entry->next->start > entry->end))) {

			/*
			 * found one.  amap creation actions do not need to
			 * be undone, but the wired counts need to be restored.
			 */

			while (entry != &map->header && entry->end > start) {
				entry->wired_count--;
				entry = entry->prev;
			}
			if ((lockflags & UVM_LK_EXIT) == 0)
				vm_map_unlock(map);
			UVMHIST_LOG(maphist,"<- done (INVALID WIRE)",0,0,0,0);
			return EINVAL;
		}
		entry = entry->next;
	}

	/*
	 * Pass 2.
	 */

#ifdef DIAGNOSTIC
	timestamp_save = map->timestamp;
#endif
	vm_map_busy(map);
	vm_map_downgrade(map);

	rv = 0;
	entry = start_entry;
	while (entry != &map->header && entry->start < end) {
		if (entry->wired_count == 1) {
			rv = uvm_fault_wire(map, entry->start, entry->end,
			    VM_FAULT_WIREMAX, entry->max_protection);
			if (rv) {

				/*
				 * wiring failed.  break out of the loop.
				 * we'll clean up the map below, once we
				 * have a write lock again.
				 */

				break;
			}
		}
		entry = entry->next;
	}

	if (rv) {	/* failed? */

		/*
		 * Get back to an exclusive (write) lock.
		 */

		vm_map_upgrade(map);
		vm_map_unbusy(map);

#ifdef DIAGNOSTIC
		if (timestamp_save != map->timestamp)
			panic("uvm_map_pageable: stale map");
#endif

		/*
		 * first drop the wiring count on all the entries
		 * which haven't actually been wired yet.
		 */

		failed_entry = entry;
		while (entry != &map->header && entry->start < end) {
			entry->wired_count--;
			entry = entry->next;
		}

		/*
		 * now, unwire all the entries that were successfully
		 * wired above.
		 */

		entry = start_entry;
		while (entry != failed_entry) {
			entry->wired_count--;
			if (VM_MAPENT_ISWIRED(entry) == 0)
				uvm_map_entry_unwire(map, entry);
			entry = entry->next;
		}
		if ((lockflags & UVM_LK_EXIT) == 0)
			vm_map_unlock(map);
		UVMHIST_LOG(maphist, "<- done (RV=%d)", rv,0,0,0);
		return (rv);
	}

	/* We are holding a read lock here. */
	if ((lockflags & UVM_LK_EXIT) == 0) {
		vm_map_unbusy(map);
		vm_map_unlock_read(map);
	} else {

		/*
		 * Get back to an exclusive (write) lock.
		 */

		vm_map_upgrade(map);
		vm_map_unbusy(map);
	}

	UVMHIST_LOG(maphist,"<- done (OK WIRE)",0,0,0,0);
	return 0;
}

/*
 * uvm_map_pageable_all: special case of uvm_map_pageable - affects
 * all mapped regions.
 *
 * => map must not be locked.
 * => if no flags are specified, all regions are unwired.
 * => XXXJRT: has some of the same problems as uvm_map_pageable() above.
 */

int
uvm_map_pageable_all(struct vm_map *map, int flags, vsize_t limit)
{
	struct vm_map_entry *entry, *failed_entry;
	vsize_t size;
	int rv;
#ifdef DIAGNOSTIC
	u_int timestamp_save;
#endif
	UVMHIST_FUNC("uvm_map_pageable_all"); UVMHIST_CALLED(maphist);
	UVMHIST_LOG(maphist,"(map=0x%x,flags=0x%x)", map, flags, 0, 0);

	KASSERT(map->flags & VM_MAP_PAGEABLE);

	vm_map_lock(map);

	/*
	 * handle wiring and unwiring separately.
	 */

	if (flags == 0) {			/* unwire */

		/*
		 * POSIX 1003.1b -- munlockall unlocks all regions,
		 * regardless of how many times mlockall has been called.
		 */

		for (entry = map->header.next; entry != &map->header;
		     entry = entry->next) {
			if (VM_MAPENT_ISWIRED(entry))
				uvm_map_entry_unwire(map, entry);
		}
		vm_map_modflags(map, 0, VM_MAP_WIREFUTURE);
		vm_map_unlock(map);
		UVMHIST_LOG(maphist,"<- done (OK UNWIRE)",0,0,0,0);
		return 0;
	}

	if (flags & MCL_FUTURE) {

		/*
		 * must wire all future mappings; remember this.
		 */

		vm_map_modflags(map, VM_MAP_WIREFUTURE, 0);
	}

	if ((flags & MCL_CURRENT) == 0) {

		/*
		 * no more work to do!
		 */

		UVMHIST_LOG(maphist,"<- done (OK no wire)",0,0,0,0);
		vm_map_unlock(map);
		return 0;
	}

	/*
	 * wire case: in three passes [XXXCDC: ugly block of code here]
	 *
	 * 1: holding the write lock, count all pages mapped by non-wired
	 *    entries.  if this would cause us to go over our limit, we fail.
	 *
	 * 2: still holding the write lock, we create any anonymous maps that
	 *    need to be created.  then we increment its wiring count.
	 *
	 * 3: we downgrade to a read lock, and call uvm_fault_wire to fault
	 *    in the pages for any newly wired area (wired_count == 1).
	 *
	 *    downgrading to a read lock for uvm_fault_wire avoids a possible
	 *    deadlock with another thread that may have faulted on one of
	 *    the pages to be wired (it would mark the page busy, blocking
	 *    us, then in turn block on the map lock that we hold).  because
	 *    of problems in the recursive lock package, we cannot upgrade
	 *    to a write lock in vm_map_lookup.  thus, any actions that
	 *    require the write lock must be done beforehand.  because we
	 *    keep the read lock on the map, the copy-on-write status of the
	 *    entries we modify here cannot change.
	 */

	for (size = 0, entry = map->header.next; entry != &map->header;
	     entry = entry->next) {
		if (entry->protection != VM_PROT_NONE &&
		    VM_MAPENT_ISWIRED(entry) == 0) { /* not already wired? */
			size += entry->end - entry->start;
		}
	}

	if (atop(size) + uvmexp.wired > uvmexp.wiredmax) {
		vm_map_unlock(map);
		return ENOMEM;
	}

	if (limit != 0 &&
	    (size + ptoa(pmap_wired_count(vm_map_pmap(map))) > limit)) {
		vm_map_unlock(map);
		return ENOMEM;
	}

	/*
	 * Pass 2.
	 */

	for (entry = map->header.next; entry != &map->header;
	     entry = entry->next) {
		if (entry->protection == VM_PROT_NONE)
			continue;
		if (VM_MAPENT_ISWIRED(entry) == 0) { /* not already wired? */

			/*
			 * perform actions of vm_map_lookup that need the
			 * write lock on the map: create an anonymous map
			 * for a copy-on-write region, or an anonymous map
			 * for a zero-fill region.  (XXXCDC: submap case
			 * ok?)
			 */

			if (!UVM_ET_ISSUBMAP(entry)) {	/* not submap */
				if (UVM_ET_ISNEEDSCOPY(entry) &&
				    ((entry->max_protection & VM_PROT_WRITE) ||
				     (entry->object.uvm_obj == NULL))) {
					amap_copy(map, entry, M_WAITOK, TRUE,
					    entry->start, entry->end);
					/* XXXCDC: wait OK? */
				}
			}
		}
		entry->wired_count++;
	}

	/*
	 * Pass 3.
	 */

#ifdef DIAGNOSTIC
	timestamp_save = map->timestamp;
#endif
	vm_map_busy(map);
	vm_map_downgrade(map);

	rv = 0;
	for (entry = map->header.next; entry != &map->header;
	     entry = entry->next) {
		if (entry->wired_count == 1) {
			rv = uvm_fault_wire(map, entry->start, entry->end,
			    VM_FAULT_WIREMAX, entry->max_protection);
			if (rv) {

				/*
				 * wiring failed.  break out of the loop.
				 * we'll clean up the map below, once we
				 * have a write lock again.
				 */

				break;
			}
		}
	}

	if (rv) {

		/*
		 * Get back an exclusive (write) lock.
		 */

		vm_map_upgrade(map);
		vm_map_unbusy(map);

#ifdef DIAGNOSTIC
		if (timestamp_save != map->timestamp)
			panic("uvm_map_pageable_all: stale map");
#endif

		/*
		 * first drop the wiring count on all the entries
		 * which haven't actually been wired yet.
		 *
		 * Skip VM_PROT_NONE entries like we did above.
		 */

		failed_entry = entry;
		for (/* nothing */; entry != &map->header;
		     entry = entry->next) {
			if (entry->protection == VM_PROT_NONE)
				continue;
			entry->wired_count--;
		}

		/*
		 * now, unwire all the entries that were successfully
		 * wired above.
		 *
		 * Skip VM_PROT_NONE entries like we did above.
		 */

		for (entry = map->header.next; entry != failed_entry;
		     entry = entry->next) {
			if (entry->protection == VM_PROT_NONE)
				continue;
			entry->wired_count--;
			if (VM_MAPENT_ISWIRED(entry))
				uvm_map_entry_unwire(map, entry);
		}
		vm_map_unlock(map);
		UVMHIST_LOG(maphist,"<- done (RV=%d)", rv,0,0,0);
		return (rv);
	}

	/* We are holding a read lock here. */
	vm_map_unbusy(map);
	vm_map_unlock_read(map);

	UVMHIST_LOG(maphist,"<- done (OK WIRE)",0,0,0,0);
	return 0;
}

/*
 * uvm_map_clean: clean out a map range
 *
 * => valid flags:
 *   if (flags & PGO_CLEANIT): dirty pages are cleaned first
 *   if (flags & PGO_SYNCIO): dirty pages are written synchronously
 *   if (flags & PGO_DEACTIVATE): any cached pages are deactivated after clean
 *   if (flags & PGO_FREE): any cached pages are freed after clean
 * => returns an error if any part of the specified range isn't mapped
 * => never a need to flush amap layer since the anonymous memory has
 *	no permanent home, but may deactivate pages there
 * => called from sys_msync() and sys_madvise()
 * => caller must not write-lock map (read OK).
 * => we may sleep while cleaning if SYNCIO [with map read-locked]
 */

int
uvm_map_clean(struct vm_map *map, vaddr_t start, vaddr_t end, int flags)
{
	struct vm_map_entry *current, *entry;
	struct uvm_object *uobj;
	struct vm_amap *amap;
	struct vm_anon *anon;
	struct vm_page *pg;
	vaddr_t offset;
	vsize_t size;
	voff_t uoff;
	int error, refs;
	UVMHIST_FUNC("uvm_map_clean"); UVMHIST_CALLED(maphist);

	UVMHIST_LOG(maphist,"(map=0x%x,start=0x%x,end=0x%x,flags=0x%x)",
		    map, start, end, flags);
	KASSERT((flags & (PGO_FREE|PGO_DEACTIVATE)) !=
		(PGO_FREE|PGO_DEACTIVATE));

	vm_map_lock_read(map);
	VM_MAP_RANGE_CHECK(map, start, end);
	if (uvm_map_lookup_entry(map, start, &entry) == FALSE) {
		vm_map_unlock_read(map);
		return EFAULT;
	}

	/*
	 * Make a first pass to check for holes and wiring problems.
	 */

	for (current = entry; current->start < end; current = current->next) {
		if (UVM_ET_ISSUBMAP(current)) {
			vm_map_unlock_read(map);
			return EINVAL;
		}
		if ((flags & PGO_FREE) != 0 && VM_MAPENT_ISWIRED(entry)) {
			vm_map_unlock_read(map);
			return EBUSY;
		}
		if (end <= current->end) {
			break;
		}
		if (current->end != current->next->start) {
			vm_map_unlock_read(map);
			return EFAULT;
		}
	}

	error = 0;
	for (current = entry; start < end; current = current->next) {
		amap = current->aref.ar_amap;	/* top layer */
		uobj = current->object.uvm_obj;	/* bottom layer */
		KASSERT(start >= current->start);

		/*
		 * No amap cleaning necessary if:
		 *
		 *	(1) There's no amap.
		 *
		 *	(2) We're not deactivating or freeing pages.
		 */

		if (amap == NULL || (flags & (PGO_DEACTIVATE|PGO_FREE)) == 0)
			goto flush_object;

		amap_lock(amap);
		offset = start - current->start;
		size = MIN(end, current->end) - start;
		for ( ; size != 0; size -= PAGE_SIZE, offset += PAGE_SIZE) {
			anon = amap_lookup(&current->aref, offset);
			if (anon == NULL)
				continue;

			simple_lock(&anon->an_lock);
			pg = anon->u.an_page;
			if (pg == NULL) {
				simple_unlock(&anon->an_lock);
				continue;
			}

			switch (flags & (PGO_CLEANIT|PGO_FREE|PGO_DEACTIVATE)) {

			/*
			 * In these first 3 cases, we just deactivate the page.
			 */

			case PGO_CLEANIT|PGO_FREE:
			case PGO_CLEANIT|PGO_DEACTIVATE:
			case PGO_DEACTIVATE:
 deactivate_it:
				/*
				 * skip the page if it's loaned or wired,
				 * since it shouldn't be on a paging queue
				 * at all in these cases.
				 */

				uvm_lock_pageq();
				if (pg->loan_count != 0 ||
				    pg->wire_count != 0) {
					uvm_unlock_pageq();
					simple_unlock(&anon->an_lock);
					continue;
				}
				KASSERT(pg->uanon == anon);
				pmap_clear_reference(pg);
				uvm_pagedeactivate(pg);
				uvm_unlock_pageq();
				simple_unlock(&anon->an_lock);
				continue;

			case PGO_FREE:

				/*
				 * If there are multiple references to
				 * the amap, just deactivate the page.
				 */

				if (amap_refs(amap) > 1)
					goto deactivate_it;

				/* skip the page if it's wired */
				if (pg->wire_count != 0) {
					simple_unlock(&anon->an_lock);
					continue;
				}
				amap_unadd(&current->aref, offset);
				refs = --anon->an_ref;
				simple_unlock(&anon->an_lock);
				if (refs == 0)
					uvm_anfree(anon);
				continue;
			}
		}
		amap_unlock(amap);

 flush_object:
		/*
		 * flush pages if we've got a valid backing object.
		 * note that we must always clean object pages before
		 * freeing them since otherwise we could reveal stale
		 * data from files.
		 */

		uoff = current->offset + (start - current->start);
		size = MIN(end, current->end) - start;
		if (uobj != NULL) {
			simple_lock(&uobj->vmobjlock);
			if (uobj->pgops->pgo_put != NULL)
				error = (uobj->pgops->pgo_put)(uobj, uoff,
				    uoff + size, flags | PGO_CLEANIT);
			else
				error = 0;
		}
		start += size;
	}
	vm_map_unlock_read(map);
	return (error);
}


/*
 * uvm_map_checkprot: check protection in map
 *
 * => must allow specified protection in a fully allocated region.
 * => map must be read or write locked by caller.
 */

boolean_t
uvm_map_checkprot(struct vm_map *map, vaddr_t start, vaddr_t end,
    vm_prot_t protection)
{
	struct vm_map_entry *entry;
	struct vm_map_entry *tmp_entry;

	if (!uvm_map_lookup_entry(map, start, &tmp_entry)) {
		return (FALSE);
	}
	entry = tmp_entry;
	while (start < end) {
		if (entry == &map->header) {
			return (FALSE);
		}

		/*
		 * no holes allowed
		 */

		if (start < entry->start) {
			return (FALSE);
		}

		/*
		 * check protection associated with entry
		 */

		if ((entry->protection & protection) != protection) {
			return (FALSE);
		}
		start = entry->end;
		entry = entry->next;
	}
	return (TRUE);
}

/*
 * uvmspace_alloc: allocate a vmspace structure.
 *
 * - structure includes vm_map and pmap
 * - XXX: no locking on this structure
 * - refcnt set to 1, rest must be init'd by caller
 */
struct vmspace *
uvmspace_alloc(vaddr_t min, vaddr_t max)
{
	struct vmspace *vm;
	UVMHIST_FUNC("uvmspace_alloc"); UVMHIST_CALLED(maphist);

	vm = pool_get(&uvm_vmspace_pool, PR_WAITOK);
	uvmspace_init(vm, NULL, min, max);
	UVMHIST_LOG(maphist,"<- done (vm=0x%x)", vm,0,0,0);
	return (vm);
}

/*
 * uvmspace_init: initialize a vmspace structure.
 *
 * - XXX: no locking on this structure
 * - refcnt set to 1, rest must be init'd by caller
 */
void
uvmspace_init(struct vmspace *vm, struct pmap *pmap, vaddr_t min, vaddr_t max)
{
	UVMHIST_FUNC("uvmspace_init"); UVMHIST_CALLED(maphist);

	memset(vm, 0, sizeof(*vm));
	uvm_map_setup(&vm->vm_map, min, max, VM_MAP_PAGEABLE
#ifdef __USING_TOPDOWN_VM
	    | VM_MAP_TOPDOWN
#endif
	    );
	if (pmap)
		pmap_reference(pmap);
	else
		pmap = pmap_create();
	vm->vm_map.pmap = pmap;
	vm->vm_refcnt = 1;
	UVMHIST_LOG(maphist,"<- done",0,0,0,0);
}

/*
 * uvmspace_share: share a vmspace between two processes
 *
 * - used for vfork, threads(?)
 */

void
uvmspace_share(struct proc *p1, struct proc *p2)
{
	struct simplelock *slock = &p1->p_vmspace->vm_map.ref_lock;

	p2->p_vmspace = p1->p_vmspace;
	simple_lock(slock);
	p1->p_vmspace->vm_refcnt++;
	simple_unlock(slock);
}

/*
 * uvmspace_unshare: ensure that process "p" has its own, unshared, vmspace
 *
 * - XXX: no locking on vmspace
 */

void
uvmspace_unshare(struct lwp *l)
{
	struct proc *p = l->l_proc;
	struct vmspace *nvm, *ovm = p->p_vmspace;

	if (ovm->vm_refcnt == 1)
		/* nothing to do: vmspace isn't shared in the first place */
		return;

	/* make a new vmspace, still holding old one */
	nvm = uvmspace_fork(ovm);

	pmap_deactivate(l);		/* unbind old vmspace */
	p->p_vmspace = nvm;
	pmap_activate(l);		/* switch to new vmspace */

	uvmspace_free(ovm);		/* drop reference to old vmspace */
}

/*
 * uvmspace_exec: the process wants to exec a new program
 */

void
uvmspace_exec(struct lwp *l, vaddr_t start, vaddr_t end)
{
	struct proc *p = l->l_proc;
	struct vmspace *nvm, *ovm = p->p_vmspace;
	struct vm_map *map = &ovm->vm_map;

#ifdef __sparc__
	/* XXX cgd 960926: the sparc #ifdef should be a MD hook */
	kill_user_windows(l);   /* before stack addresses go away */
#endif

	/*
	 * see if more than one process is using this vmspace...
	 */

	if (ovm->vm_refcnt == 1) {

		/*
		 * if p is the only process using its vmspace then we can safely
		 * recycle that vmspace for the program that is being exec'd.
		 */

#ifdef SYSVSHM
		/*
		 * SYSV SHM semantics require us to kill all segments on an exec
		 */

		if (ovm->vm_shm)
			shmexit(ovm);
#endif

		/*
		 * POSIX 1003.1b -- "lock future mappings" is revoked
		 * when a process execs another program image.
		 */

		vm_map_modflags(map, 0, VM_MAP_WIREFUTURE);

		/*
		 * now unmap the old program
		 */

		pmap_remove_all(map->pmap);
		uvm_unmap(map, vm_map_min(map), vm_map_max(map));
		KASSERT(map->header.prev == &map->header);
		KASSERT(map->nentries == 0);

		/*
		 * resize the map
		 */

		vm_map_setmin(map, start);
		vm_map_setmax(map, end);
	} else {

		/*
		 * p's vmspace is being shared, so we can't reuse it for p since
		 * it is still being used for others.   allocate a new vmspace
		 * for p
		 */

		nvm = uvmspace_alloc(start, end);

		/*
		 * install new vmspace and drop our ref to the old one.
		 */

		pmap_deactivate(l);
		p->p_vmspace = nvm;
		pmap_activate(l);

		uvmspace_free(ovm);
	}
}

/*
 * uvmspace_free: free a vmspace data structure
 */

void
uvmspace_free(struct vmspace *vm)
{
	struct vm_map_entry *dead_entries;
	struct vm_map *map = &vm->vm_map;
	int n;

	UVMHIST_FUNC("uvmspace_free"); UVMHIST_CALLED(maphist);

	UVMHIST_LOG(maphist,"(vm=0x%x) ref=%d", vm, vm->vm_refcnt,0,0);
	simple_lock(&map->ref_lock);
	n = --vm->vm_refcnt;
	simple_unlock(&map->ref_lock);
	if (n > 0)
		return;

	/*
	 * at this point, there should be no other references to the map.
	 * delete all of the mappings, then destroy the pmap.
	 */

	map->flags |= VM_MAP_DYING;
	pmap_remove_all(map->pmap);
#ifdef SYSVSHM
	/* Get rid of any SYSV shared memory segments. */
	if (vm->vm_shm != NULL)
		shmexit(vm);
#endif
	if (map->nentries) {
		uvm_unmap_remove(map, vm_map_min(map), vm_map_max(map),
		    &dead_entries, NULL);
		if (dead_entries != NULL)
			uvm_unmap_detach(dead_entries, 0);
	}
	KASSERT(map->nentries == 0);
	KASSERT(map->size == 0);
	pmap_destroy(map->pmap);
	pool_put(&uvm_vmspace_pool, vm);
}

/*
 *   F O R K   -   m a i n   e n t r y   p o i n t
 */
/*
 * uvmspace_fork: fork a process' main map
 *
 * => create a new vmspace for child process from parent.
 * => parent's map must not be locked.
 */

struct vmspace *
uvmspace_fork(struct vmspace *vm1)
{
	struct vmspace *vm2;
	struct vm_map *old_map = &vm1->vm_map;
	struct vm_map *new_map;
	struct vm_map_entry *old_entry;
	struct vm_map_entry *new_entry;
	UVMHIST_FUNC("uvmspace_fork"); UVMHIST_CALLED(maphist);

	vm_map_lock(old_map);

	vm2 = uvmspace_alloc(vm_map_min(old_map), vm_map_max(old_map));
	memcpy(&vm2->vm_startcopy, &vm1->vm_startcopy,
	    (caddr_t) (vm1 + 1) - (caddr_t) &vm1->vm_startcopy);
	new_map = &vm2->vm_map;		  /* XXX */

	old_entry = old_map->header.next;
	new_map->size = old_map->size;

	/*
	 * go entry-by-entry
	 */

	while (old_entry != &old_map->header) {

		/*
		 * first, some sanity checks on the old entry
		 */

		KASSERT(!UVM_ET_ISSUBMAP(old_entry));
		KASSERT(UVM_ET_ISCOPYONWRITE(old_entry) ||
			!UVM_ET_ISNEEDSCOPY(old_entry));

		switch (old_entry->inheritance) {
		case MAP_INHERIT_NONE:

			/*
			 * drop the mapping, modify size
			 */
			new_map->size -= old_entry->end - old_entry->start;
			break;

		case MAP_INHERIT_SHARE:

			/*
			 * share the mapping: this means we want the old and
			 * new entries to share amaps and backing objects.
			 */
			/*
			 * if the old_entry needs a new amap (due to prev fork)
			 * then we need to allocate it now so that we have
			 * something we own to share with the new_entry.   [in
			 * other words, we need to clear needs_copy]
			 */

			if (UVM_ET_ISNEEDSCOPY(old_entry)) {
				/* get our own amap, clears needs_copy */
				amap_copy(old_map, old_entry, M_WAITOK, FALSE,
				    0, 0);
				/* XXXCDC: WAITOK??? */
			}

			new_entry = uvm_mapent_alloc(new_map, 0);
			/* old_entry -> new_entry */
			uvm_mapent_copy(old_entry, new_entry);

			/* new pmap has nothing wired in it */
			new_entry->wired_count = 0;

			/*
			 * gain reference to object backing the map (can't
			 * be a submap, already checked this case).
			 */

			if (new_entry->aref.ar_amap)
				uvm_map_reference_amap(new_entry, AMAP_SHARED);

			if (new_entry->object.uvm_obj &&
			    new_entry->object.uvm_obj->pgops->pgo_reference)
				new_entry->object.uvm_obj->
				    pgops->pgo_reference(
				        new_entry->object.uvm_obj);

			/* insert entry at end of new_map's entry list */
			uvm_map_entry_link(new_map, new_map->header.prev,
			    new_entry);

			break;

		case MAP_INHERIT_COPY:

			/*
			 * copy-on-write the mapping (using mmap's
			 * MAP_PRIVATE semantics)
			 *
			 * allocate new_entry, adjust reference counts.
			 * (note that new references are read-only).
			 */

			new_entry = uvm_mapent_alloc(new_map, 0);
			/* old_entry -> new_entry */
			uvm_mapent_copy(old_entry, new_entry);

			if (new_entry->aref.ar_amap)
				uvm_map_reference_amap(new_entry, 0);

			if (new_entry->object.uvm_obj &&
			    new_entry->object.uvm_obj->pgops->pgo_reference)
				new_entry->object.uvm_obj->pgops->pgo_reference
				    (new_entry->object.uvm_obj);

			/* new pmap has nothing wired in it */
			new_entry->wired_count = 0;

			new_entry->etype |=
			    (UVM_ET_COPYONWRITE|UVM_ET_NEEDSCOPY);
			uvm_map_entry_link(new_map, new_map->header.prev,
			    new_entry);

			/*
			 * the new entry will need an amap.  it will either
			 * need to be copied from the old entry or created
			 * from scratch (if the old entry does not have an
			 * amap).  can we defer this process until later
			 * (by setting "needs_copy") or do we need to copy
			 * the amap now?
			 *
			 * we must copy the amap now if any of the following
			 * conditions hold:
			 * 1. the old entry has an amap and that amap is
			 *    being shared.  this means that the old (parent)
			 *    process is sharing the amap with another
			 *    process.  if we do not clear needs_copy here
			 *    we will end up in a situation where both the
			 *    parent and child process are refering to the
			 *    same amap with "needs_copy" set.  if the
			 *    parent write-faults, the fault routine will
			 *    clear "needs_copy" in the parent by allocating
			 *    a new amap.   this is wrong because the
			 *    parent is supposed to be sharing the old amap
			 *    and the new amap will break that.
			 *
			 * 2. if the old entry has an amap and a non-zero
			 *    wire count then we are going to have to call
			 *    amap_cow_now to avoid page faults in the
			 *    parent process.   since amap_cow_now requires
			 *    "needs_copy" to be clear we might as well
			 *    clear it here as well.
			 *
			 */

			if (old_entry->aref.ar_amap != NULL) {
				if ((amap_flags(old_entry->aref.ar_amap) &
				     AMAP_SHARED) != 0 ||
				    VM_MAPENT_ISWIRED(old_entry)) {

					amap_copy(new_map, new_entry, M_WAITOK,
					    FALSE, 0, 0);
					/* XXXCDC: M_WAITOK ... ok? */
				}
			}

			/*
			 * if the parent's entry is wired down, then the
			 * parent process does not want page faults on
			 * access to that memory.  this means that we
			 * cannot do copy-on-write because we can't write
			 * protect the old entry.   in this case we
			 * resolve all copy-on-write faults now, using
			 * amap_cow_now.   note that we have already
			 * allocated any needed amap (above).
			 */

			if (VM_MAPENT_ISWIRED(old_entry)) {

			  /*
			   * resolve all copy-on-write faults now
			   * (note that there is nothing to do if
			   * the old mapping does not have an amap).
			   */
			  if (old_entry->aref.ar_amap)
			    amap_cow_now(new_map, new_entry);

			} else {

			  /*
			   * setup mappings to trigger copy-on-write faults
			   * we must write-protect the parent if it has
			   * an amap and it is not already "needs_copy"...
			   * if it is already "needs_copy" then the parent
			   * has already been write-protected by a previous
			   * fork operation.
			   */

			  if (old_entry->aref.ar_amap &&
			      !UVM_ET_ISNEEDSCOPY(old_entry)) {
			      if (old_entry->max_protection & VM_PROT_WRITE) {
				pmap_protect(old_map->pmap,
					     old_entry->start,
					     old_entry->end,
					     old_entry->protection &
					     ~VM_PROT_WRITE);
				pmap_update(old_map->pmap);
			      }
			      old_entry->etype |= UVM_ET_NEEDSCOPY;
			  }
			}
			break;
		}  /* end of switch statement */
		old_entry = old_entry->next;
	}

	vm_map_unlock(old_map);

#ifdef SYSVSHM
	if (vm1->vm_shm)
		shmfork(vm1, vm2);
#endif

#ifdef PMAP_FORK
	pmap_fork(vm1->vm_map.pmap, vm2->vm_map.pmap);
#endif

	UVMHIST_LOG(maphist,"<- done",0,0,0,0);
	return (vm2);
}


/*
 * in-kernel map entry allocation.
 */

int ukh_alloc, ukh_free;
int uke_alloc, uke_free;

struct uvm_kmapent_hdr {
	LIST_ENTRY(uvm_kmapent_hdr) ukh_listq;
	int ukh_nused;
	struct vm_map_entry *ukh_freelist;
	struct vm_map *ukh_map;
	struct vm_map_entry ukh_entries[0];
};

#define	UVM_KMAPENT_CHUNK				\
	((PAGE_SIZE - sizeof(struct uvm_kmapent_hdr))	\
	/ sizeof(struct vm_map_entry))

#define	UVM_KHDR_FIND(entry)	\
	((struct uvm_kmapent_hdr *)(((vaddr_t)entry) & ~PAGE_MASK))

static __inline struct vm_map_entry *uvm_kmapent_get(struct uvm_kmapent_hdr *);
static __inline void uvm_kmapent_put(struct uvm_kmapent_hdr *,
    struct vm_map_entry *);

static __inline struct vm_map *
uvm_kmapent_map(struct vm_map_entry *entry)
{
	const struct uvm_kmapent_hdr *ukh;

	ukh = UVM_KHDR_FIND(entry);
	return ukh->ukh_map;
}

static __inline struct vm_map_entry *
uvm_kmapent_get(struct uvm_kmapent_hdr *ukh)
{
	struct vm_map_entry *entry;

	KASSERT(ukh->ukh_nused <= UVM_KMAPENT_CHUNK);
	KASSERT(ukh->ukh_nused >= 0);

	entry = ukh->ukh_freelist;
	if (entry) {
		KASSERT((entry->flags & (UVM_MAP_KERNEL | UVM_MAP_KMAPENT))
		    == UVM_MAP_KERNEL);
		ukh->ukh_freelist = entry->next;
		ukh->ukh_nused++;
		KASSERT(ukh->ukh_nused <= UVM_KMAPENT_CHUNK);
	} else {
		KASSERT(ukh->ukh_nused == UVM_KMAPENT_CHUNK);
	}

	return entry;
}

static __inline void
uvm_kmapent_put(struct uvm_kmapent_hdr *ukh, struct vm_map_entry *entry)
{

	KASSERT((entry->flags & (UVM_MAP_KERNEL | UVM_MAP_KMAPENT))
	    == UVM_MAP_KERNEL);
	KASSERT(ukh->ukh_nused <= UVM_KMAPENT_CHUNK);
	KASSERT(ukh->ukh_nused > 0);
	KASSERT(ukh->ukh_freelist != NULL ||
	    ukh->ukh_nused == UVM_KMAPENT_CHUNK);
	KASSERT(ukh->ukh_freelist == NULL ||
	    ukh->ukh_nused < UVM_KMAPENT_CHUNK);

	ukh->ukh_nused--;
	entry->next = ukh->ukh_freelist;
	ukh->ukh_freelist = entry;
}

/*
 * uvm_kmapent_alloc: allocate a map entry for in-kernel map
 */

static struct vm_map_entry *
uvm_kmapent_alloc(struct vm_map *map, int flags)
{
	struct vm_page *pg;
	struct uvm_map_args args;
	struct uvm_kmapent_hdr *ukh;
	struct vm_map_entry *entry;
	uvm_flag_t mapflags = UVM_MAPFLAG(UVM_PROT_ALL, UVM_PROT_ALL,
	    UVM_INH_NONE, UVM_ADV_RANDOM, flags | UVM_FLAG_NOMERGE);
	vaddr_t va;
	int error;
	int i;
	int s;

	KDASSERT(UVM_KMAPENT_CHUNK > 2);
	KDASSERT(kernel_map != NULL);
	KASSERT(vm_map_pmap(map) == pmap_kernel());

	uke_alloc++;
	entry = NULL;
again:
	/*
	 * try to grab an entry from freelist.
	 */
	s = splvm();
	simple_lock(&uvm.kentry_lock);
	ukh = LIST_FIRST(&vm_map_to_kernel(map)->vmk_kentry_free);
	if (ukh) {
		entry = uvm_kmapent_get(ukh);
		if (ukh->ukh_nused == UVM_KMAPENT_CHUNK)
			LIST_REMOVE(ukh, ukh_listq);
	}
	simple_unlock(&uvm.kentry_lock);
	splx(s);

	if (entry)
		return entry;

	/*
	 * there's no free entry for this vm_map.
	 * now we need to allocate some vm_map_entry.
	 * for simplicity, always allocate one page chunk of them at once.
	 */

	pg = uvm_pagealloc(NULL, 0, NULL, 0);
	if (__predict_false(pg == NULL)) {
		if (flags & UVM_FLAG_NOWAIT)
			return NULL;
		uvm_wait("kme_alloc");
		goto again;
	}

	error = uvm_map_prepare(map, 0, PAGE_SIZE, NULL, 0, 0, mapflags, &args);
	if (error) {
		uvm_pagefree(pg);
		return NULL;
	}

	va = args.uma_start;

	pmap_kenter_pa(va, VM_PAGE_TO_PHYS(pg), VM_PROT_READ|VM_PROT_WRITE);
	pmap_update(vm_map_pmap(map));

	ukh = (void *)va;

	/*
	 * use the first entry for ukh itsself.
	 */

	entry = &ukh->ukh_entries[0];
	entry->flags = UVM_MAP_KERNEL | UVM_MAP_KMAPENT;
	error = uvm_map_enter(map, &args, entry);
	KASSERT(error == 0);

	ukh->ukh_nused = UVM_KMAPENT_CHUNK;
	ukh->ukh_map = map;
	ukh->ukh_freelist = NULL;
	for (i = UVM_KMAPENT_CHUNK - 1; i >= 2; i--) {
		struct vm_map_entry *entry = &ukh->ukh_entries[i];

		entry->flags = UVM_MAP_KERNEL;
		uvm_kmapent_put(ukh, entry);
	}
	KASSERT(ukh->ukh_nused == 2);

	s = splvm();
	simple_lock(&uvm.kentry_lock);
	LIST_INSERT_HEAD(&vm_map_to_kernel(map)->vmk_kentry_free,
	    ukh, ukh_listq);
	simple_unlock(&uvm.kentry_lock);
	splx(s);

	/*
	 * return second entry.
	 */

	entry = &ukh->ukh_entries[1];
	entry->flags = UVM_MAP_KERNEL;
	ukh_alloc++;
	return entry;
}

/*
 * uvm_mapent_free: free map entry for in-kernel map
 */

static void
uvm_kmapent_free(struct vm_map_entry *entry)
{
	struct uvm_kmapent_hdr *ukh;
	struct vm_page *pg;
	struct vm_map *map;
	struct pmap *pmap;
	vaddr_t va;
	paddr_t pa;
	struct vm_map_entry *deadentry;
	int s;

	uke_free++;
	ukh = UVM_KHDR_FIND(entry);
	map = ukh->ukh_map;

	s = splvm();
	simple_lock(&uvm.kentry_lock);
	uvm_kmapent_put(ukh, entry);
	if (ukh->ukh_nused > 1) {
		if (ukh->ukh_nused == UVM_KMAPENT_CHUNK - 1)
			LIST_INSERT_HEAD(
			    &vm_map_to_kernel(map)->vmk_kentry_free,
			    ukh, ukh_listq);
		simple_unlock(&uvm.kentry_lock);
		splx(s);
		return;
	}

	/*
	 * now we can free this ukh.
	 *
	 * however, keep an empty ukh to avoid ping-pong.
	 */

	if (LIST_FIRST(&vm_map_to_kernel(map)->vmk_kentry_free) == ukh &&
	    LIST_NEXT(ukh, ukh_listq) == NULL) {
		simple_unlock(&uvm.kentry_lock);
		splx(s);
		return;
	}
	LIST_REMOVE(ukh, ukh_listq);
	simple_unlock(&uvm.kentry_lock);
	splx(s);

	KASSERT(ukh->ukh_nused == 1);

	/*
	 * remove map entry for ukh itsself.
	 */

	va = (vaddr_t)ukh;
	KASSERT((va & PAGE_MASK) == 0);
	uvm_unmap_remove(map, va, va + PAGE_SIZE, &deadentry, NULL);
	KASSERT(deadentry->flags & UVM_MAP_KERNEL);
	KASSERT(deadentry->flags & UVM_MAP_KMAPENT);
	KASSERT(deadentry->next == NULL);
	KASSERT(deadentry == &ukh->ukh_entries[0]);

	/*
	 * unmap the page from pmap and free it.
	 */

	pmap = vm_map_pmap(map);
	KASSERT(pmap == pmap_kernel());
	if (!pmap_extract(pmap, va, &pa))
		panic("%s: no mapping", __func__);
	pmap_kremove(va, PAGE_SIZE);
	pg = PHYS_TO_VM_PAGE(pa);
	uvm_pagefree(pg);
	ukh_free++;
}

/*
 * map entry reservation
 */

/*
 * uvm_mapent_reserve: reserve map entries for clipping before locking map.
 *
 * => needed when unmapping entries allocated without UVM_FLAG_QUANTUM.
 * => caller shouldn't hold map locked.
 */
int
uvm_mapent_reserve(struct vm_map *map, struct uvm_mapent_reservation *umr,
    int nentries, int flags)
{

	umr->umr_nentries = 0;

	if ((flags & UVM_FLAG_QUANTUM) != 0)
		return 0;

	if (!VM_MAP_USE_KMAPENT(map))
		return 0;

	while (nentries--) {
		struct vm_map_entry *ent;
		ent = uvm_kmapent_alloc(map, flags);
		if (!ent) {
			uvm_mapent_unreserve(map, umr);
			return ENOMEM;
		}
		UMR_PUTENTRY(umr, ent);
	}

	return 0;
}

/*
 * uvm_mapent_unreserve:
 *
 * => caller shouldn't hold map locked.
 * => never fail or sleep.
 */
void
uvm_mapent_unreserve(struct vm_map *map, struct uvm_mapent_reservation *umr)
{

	while (!UMR_EMPTY(umr))
		uvm_kmapent_free(UMR_GETENTRY(umr));
}

#if defined(DDB)

/*
 * DDB hooks
 */

/*
 * uvm_map_printit: actually prints the map
 */

void
uvm_map_printit(struct vm_map *map, boolean_t full,
    void (*pr)(const char *, ...))
{
	struct vm_map_entry *entry;

	(*pr)("MAP %p: [0x%lx->0x%lx]\n", map, vm_map_min(map),
	    vm_map_max(map));
	(*pr)("\t#ent=%d, sz=%d, ref=%d, version=%d, flags=0x%x\n",
	    map->nentries, map->size, map->ref_count, map->timestamp,
	    map->flags);
	(*pr)("\tpmap=%p(resident=%ld, wired=%ld)\n", map->pmap,
	    pmap_resident_count(map->pmap), pmap_wired_count(map->pmap));
	if (!full)
		return;
	for (entry = map->header.next; entry != &map->header;
	    entry = entry->next) {
		(*pr)(" - %p: 0x%lx->0x%lx: obj=%p/0x%llx, amap=%p/%d\n",
		    entry, entry->start, entry->end, entry->object.uvm_obj,
		    (long long)entry->offset, entry->aref.ar_amap,
		    entry->aref.ar_pageoff);
		(*pr)(
		    "\tsubmap=%c, cow=%c, nc=%c, prot(max)=%d/%d, inh=%d, "
		    "wc=%d, adv=%d\n",
		    (entry->etype & UVM_ET_SUBMAP) ? 'T' : 'F',
		    (entry->etype & UVM_ET_COPYONWRITE) ? 'T' : 'F',
		    (entry->etype & UVM_ET_NEEDSCOPY) ? 'T' : 'F',
		    entry->protection, entry->max_protection,
		    entry->inheritance, entry->wired_count, entry->advice);
	}
}

/*
 * uvm_object_printit: actually prints the object
 */

void
uvm_object_printit(struct uvm_object *uobj, boolean_t full,
    void (*pr)(const char *, ...))
{
	struct vm_page *pg;
	int cnt = 0;

	(*pr)("OBJECT %p: locked=%d, pgops=%p, npages=%d, ",
	    uobj, uobj->vmobjlock.lock_data, uobj->pgops, uobj->uo_npages);
	if (UVM_OBJ_IS_KERN_OBJECT(uobj))
		(*pr)("refs=<SYSTEM>\n");
	else
		(*pr)("refs=%d\n", uobj->uo_refs);

	if (!full) {
		return;
	}
	(*pr)("  PAGES <pg,offset>:\n  ");
	TAILQ_FOREACH(pg, &uobj->memq, listq) {
		cnt++;
		(*pr)("<%p,0x%llx> ", pg, (long long)pg->offset);
		if ((cnt % 3) == 0) {
			(*pr)("\n  ");
		}
	}
	if ((cnt % 3) != 0) {
		(*pr)("\n");
	}
}

/*
 * uvm_page_printit: actually print the page
 */

static const char page_flagbits[] =
	"\20\1BUSY\2WANTED\3TABLED\4CLEAN\5PAGEOUT\6RELEASED\7FAKE\10RDONLY"
	"\11ZERO\15PAGER1";
static const char page_pqflagbits[] =
	"\20\1FREE\2INACTIVE\3ACTIVE\5ANON\6AOBJ";

void
uvm_page_printit(struct vm_page *pg, boolean_t full,
    void (*pr)(const char *, ...))
{
	struct vm_page *tpg;
	struct uvm_object *uobj;
	struct pglist *pgl;
	char pgbuf[128];
	char pqbuf[128];

	(*pr)("PAGE %p:\n", pg);
	bitmask_snprintf(pg->flags, page_flagbits, pgbuf, sizeof(pgbuf));
	bitmask_snprintf(pg->pqflags, page_pqflagbits, pqbuf, sizeof(pqbuf));
	(*pr)("  flags=%s, pqflags=%s, wire_count=%d, pa=0x%lx\n",
	    pgbuf, pqbuf, pg->wire_count, (long)VM_PAGE_TO_PHYS(pg));
	(*pr)("  uobject=%p, uanon=%p, offset=0x%llx loan_count=%d\n",
	    pg->uobject, pg->uanon, (long long)pg->offset, pg->loan_count);
#if defined(UVM_PAGE_TRKOWN)
	if (pg->flags & PG_BUSY)
		(*pr)("  owning process = %d, tag=%s\n",
		    pg->owner, pg->owner_tag);
	else
		(*pr)("  page not busy, no owner\n");
#else
	(*pr)("  [page ownership tracking disabled]\n");
#endif

	if (!full)
		return;

	/* cross-verify object/anon */
	if ((pg->pqflags & PQ_FREE) == 0) {
		if (pg->pqflags & PQ_ANON) {
			if (pg->uanon == NULL || pg->uanon->u.an_page != pg)
			    (*pr)("  >>> ANON DOES NOT POINT HERE <<< (%p)\n",
				(pg->uanon) ? pg->uanon->u.an_page : NULL);
			else
				(*pr)("  anon backpointer is OK\n");
		} else {
			uobj = pg->uobject;
			if (uobj) {
				(*pr)("  checking object list\n");
				TAILQ_FOREACH(tpg, &uobj->memq, listq) {
					if (tpg == pg) {
						break;
					}
				}
				if (tpg)
					(*pr)("  page found on object list\n");
				else
			(*pr)("  >>> PAGE NOT FOUND ON OBJECT LIST! <<<\n");
			}
		}
	}

	/* cross-verify page queue */
	if (pg->pqflags & PQ_FREE) {
		int fl = uvm_page_lookup_freelist(pg);
		int color = VM_PGCOLOR_BUCKET(pg);
		pgl = &uvm.page_free[fl].pgfl_buckets[color].pgfl_queues[
		    ((pg)->flags & PG_ZERO) ? PGFL_ZEROS : PGFL_UNKNOWN];
	} else if (pg->pqflags & PQ_INACTIVE) {
		pgl = &uvm.page_inactive;
	} else if (pg->pqflags & PQ_ACTIVE) {
		pgl = &uvm.page_active;
	} else {
		pgl = NULL;
	}

	if (pgl) {
		(*pr)("  checking pageq list\n");
		TAILQ_FOREACH(tpg, pgl, pageq) {
			if (tpg == pg) {
				break;
			}
		}
		if (tpg)
			(*pr)("  page found on pageq list\n");
		else
			(*pr)("  >>> PAGE NOT FOUND ON PAGEQ LIST! <<<\n");
	}
}
#endif
