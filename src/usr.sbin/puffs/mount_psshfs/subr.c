/*      $NetBSD: subr.c,v 1.4 2007/01/09 12:34:20 pooka Exp $        */
        
/*      
 * Copyright (c) 2006  Antti Kantee.  All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:     
 * 1. Redistributions of source code must retain the above copyright  
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the company nor the name of the author may be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 *      
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */     
        
#include <sys/cdefs.h>
#ifndef lint
__RCSID("$NetBSD: subr.c,v 1.4 2007/01/09 12:34:20 pooka Exp $");
#endif /* !lint */

#include <assert.h>
#include <errno.h>
#include <puffs.h>
#include <stdlib.h>
#include <util.h>

#include "psshfs.h"
#include "sftp_proto.h"

static void
freedircache(struct psshfs_dir *base, size_t count)
{
	int i;

	for (i = 0; i < count; i++) {
		free(base[i].entryname);
		base[i].entryname = NULL;
	}

	free(base);
}

#define ENTRYCHUNK 16
static void
allocdirs(struct psshfs_node *psn)
{
	size_t oldtot = psn->denttot;

	psn->denttot += ENTRYCHUNK;
	psn->dir = erealloc(psn->dir,
	    psn->denttot * sizeof(struct psshfs_dir));
	memset(psn->dir + oldtot, 0, ENTRYCHUNK * sizeof(struct psshfs_dir));
}

struct psshfs_dir *
lookup(struct psshfs_dir *basedir, size_t ndir, const char *name)
{
	struct psshfs_dir *ent;
	int i;

	for (i = 0; i < ndir; i++) {
		ent = &basedir[i];
		if (ent->valid != 1)
			continue;
		if (strcmp(ent->entryname, name) == 0)
			return ent;
	}

	return NULL;
}

int
sftp_readdir(struct puffs_cc *pcc, struct psshfs_ctx *pctx,
	struct puffs_node *pn)
{
	struct psshfs_node *psn = pn->pn_data;
	struct psshfs_dir *olddir, *testd;
	struct psbuf *pb;
	uint32_t reqid = NEXTREQ(pctx);
	uint32_t count;
	char *dhand = NULL;
	size_t dhandlen, nent;
	char *longname;
	int idx, rv;

	assert(pn->pn_va.va_type == VDIR);

	if (psn->dir && (time(NULL) - psn->dentread) < PSSHFS_REFRESHIVAL)
		return 0;

	pb = psbuf_make(PSB_OUT);
	psbuf_req_str(pb, SSH_FXP_OPENDIR, reqid, pn->pn_path);
	pssh_outbuf_enqueue(pctx, pb, pcc, reqid);

	puffs_cc_yield(pcc);

	rv = psbuf_expect_handle(pb, &dhand, &dhandlen);
	if (rv)
		goto wayout;

	/*
	 * Well, the following is O(n^2), so feel free to improve if it
	 * gets too taxing on your system.
	 */
	olddir = psn->dir;
	nent = psn->dentnext;

	psn->dentnext = 0;
	psn->denttot = 0;
	psn->dir = NULL;
	idx = 0;

	for (;;) {
		reqid = NEXTREQ(pctx);
		psbuf_recycle(pb, PSB_OUT);
		psbuf_req_data(pb, SSH_FXP_READDIR, reqid, dhand, dhandlen);
		pssh_outbuf_enqueue(pctx, pb, pcc, reqid);

		puffs_cc_yield(pcc);

		/* check for EOF */
		if (pb->type == SSH_FXP_STATUS) {
			rv = psbuf_expect_status(pb);
			goto out;
		}
		rv = psbuf_expect_name(pb, &count);
		if (rv)
			goto out;

		for (; count--; idx++) {
			if (idx == psn->denttot)
				allocdirs(psn);
			if (!psbuf_get_str(pb, &psn->dir[idx].entryname,
			    NULL)) {
				rv = EPROTO;
				goto out;
			}
			if (!psbuf_get_str(pb, &longname, NULL)) {
				rv = EPROTO;
				goto out;
			}
			if (!psbuf_get_vattr(pb, &psn->dir[idx].va)) {
				rv = EPROTO;
				goto out;
			}
			if (sscanf(longname, "%*s%d",
			    &psn->dir[idx].va.va_nlink) != 1) {
				rv = EPROTO;
				goto out;
			}
			free(longname);

			testd = lookup(olddir, nent, psn->dir[idx].entryname);
			if (testd) {
				psn->dir[idx].entry = testd->entry;
				psn->dir[idx].va.va_fileid
				    = testd->va.va_fileid;
			} else {
				psn->dir[idx].entry = NULL;
				psn->dir[idx].va.va_fileid = pctx->nextino++;
			}
			psn->dir[idx].valid = 1;
		}
	}

 out:
	/* XXX: rv */
	psn->dentnext = idx;
	psn->dentread = time(NULL);
	freedircache(olddir, nent);

	reqid = NEXTREQ(pctx);
	psbuf_recycle(pb, PSB_OUT);
	psbuf_req_data(pb, SSH_FXP_CLOSE, reqid, dhand, dhandlen);
	pssh_outbuf_enqueue(pctx, pb, pcc, reqid);

	puffs_cc_yield(pcc);

	/* EDONTCARE for the response */

 wayout:
	free(dhand);
	psbuf_destroy(pb);
	return rv;
}

struct puffs_node *
makenode(struct puffs_usermount *pu, struct puffs_node *parent,
	struct psshfs_dir *pd, const struct vattr *vap)
{
	struct psshfs_node *psn_parent = parent->pn_data;
	struct psshfs_node *psn;
	struct puffs_node *pn;

	psn = emalloc(sizeof(struct psshfs_node));
	memset(psn, 0, sizeof(struct psshfs_node));

	pn = puffs_pn_new(pu, psn);
	if (!pn) {
		free(psn);
		return NULL;
	}
	puffs_setvattr(&pn->pn_va, &pd->va);
	puffs_setvattr(&pn->pn_va, vap);

	pd->entry = pn;
	psn->parent = parent;
	psn_parent->childcount++;

	return pn;
}

struct puffs_node *
allocnode(struct puffs_usermount *pu, struct puffs_node *parent,
	const char *entryname, const struct vattr *vap)
{
	struct psshfs_ctx *pctx = pu->pu_privdata;
	struct psshfs_dir *pd;
	struct puffs_node *pn;

	pd = direnter(parent, entryname);

	pd->va.va_fileid = pctx->nextino++;
	if (vap->va_type == VDIR) {
		pd->va.va_nlink = 2;
		parent->pn_va.va_nlink++;
	} else {
		pd->va.va_nlink = 1;
	}

	pn = makenode(pu, parent, pd, vap);
	if (pn)
		pd->va.va_fileid = pn->pn_va.va_fileid;

	return pn;
}

struct psshfs_dir *
direnter(struct puffs_node *parent, const char *entryname)
{
	struct psshfs_node *psn_parent = parent->pn_data;
	struct psshfs_dir *pd;
	int i;

	/* create directory entry */
	if (psn_parent->denttot == psn_parent->dentnext)
		allocdirs(psn_parent);

	i = psn_parent->dentnext;
	pd = &psn_parent->dir[i];
	pd->entryname = estrdup(entryname);
	pd->valid = 1;
	puffs_vattr_null(&pd->va);
	psn_parent->dentnext++;

	return pd;
}

void
nukenode(struct puffs_node *node, const char *entryname)
{
	struct psshfs_node *psn, *psn_parent;
	struct psshfs_dir *pd;

	psn = node->pn_data;
	psn_parent = psn->parent->pn_data;
	psn_parent->childcount--;
	pd = lookup(psn_parent->dir, psn_parent->dentnext, entryname);
	assert(pd != NULL);
	pd->valid = 0;
	free(pd->entryname);
	pd->entryname = NULL;

	if (node->pn_va.va_type == VDIR) {
		psn->parent->pn_va.va_nlink--;
		freedircache(psn->dir, psn->dentnext);
	}
}
