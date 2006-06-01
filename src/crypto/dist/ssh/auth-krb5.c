/*	$NetBSD: auth-krb5.c,v 1.15 2005/02/13 05:57:26 christos Exp $	*/
/*
 *    Kerberos v5 authentication and ticket-passing routines.
 *
 * $FreeBSD: src/crypto/openssh/auth-krb5.c,v 1.6 2001/02/13 16:58:04 assar Exp $
 */
/*
 * Copyright (c) 2002 Daniel Kouril.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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
 */

#include "includes.h"
RCSID("$OpenBSD: auth-krb5.c,v 1.15 2003/11/21 11:57:02 djm Exp $");
__RCSID("$NetBSD: auth-krb5.c,v 1.15 2005/02/13 05:57:26 christos Exp $");

#include "ssh.h"
#include "ssh1.h"
#include "packet.h"
#include "xmalloc.h"
#include "log.h"
#include "servconf.h"
#include "uidswap.h"
#include "auth.h"

#ifdef KRB5
#include <krb5.h>

extern ServerOptions	 options;

static int
krb5_init(void *context)
{
	Authctxt *authctxt = (Authctxt *)context;
	krb5_error_code problem;

	if (authctxt->krb5_ctx == NULL) {
		problem = krb5_init_context(&authctxt->krb5_ctx);
		if (problem)
			return (problem);
		krb5_init_ets(authctxt->krb5_ctx);
	}
	return (0);
}

/*
 * Try krb5 authentication. server_user is passed for logging purposes
 * only, in auth is received ticket, in client is returned principal
 * from the ticket
 */
int
auth_krb5(Authctxt *authctxt, krb5_data *auth, char **client, krb5_data *reply)
{
	krb5_error_code problem;
	krb5_principal server;
	krb5_ticket *ticket;
	int fd, ret;

	ret = 0;
	server = NULL;
	ticket = NULL;
	reply->length = 0;

	problem = krb5_init(authctxt);
	if (problem)
		goto err;

	problem = krb5_auth_con_init(authctxt->krb5_ctx,
	    &authctxt->krb5_auth_ctx);
	if (problem)
		goto err;

	fd = packet_get_connection_in();
	problem = krb5_auth_con_setaddrs_from_fd(authctxt->krb5_ctx,
	    authctxt->krb5_auth_ctx, &fd);
	if (problem)
		goto err;

	problem = krb5_sname_to_principal(authctxt->krb5_ctx, NULL, NULL,
	    KRB5_NT_SRV_HST, &server);
	if (problem)
		goto err;

	problem = krb5_rd_req(authctxt->krb5_ctx, &authctxt->krb5_auth_ctx,
	    auth, server, NULL, NULL, &ticket);
	if (problem)
		goto err;

	problem = krb5_copy_principal(authctxt->krb5_ctx, ticket->client,
	    &authctxt->krb5_user);
	if (problem)
		goto err;

	/* if client wants mutual auth */
	problem = krb5_mk_rep(authctxt->krb5_ctx, authctxt->krb5_auth_ctx,
	    reply);
	if (problem)
		goto err;

	/* Check .k5login authorization now. */
	if (!krb5_kuserok(authctxt->krb5_ctx, authctxt->krb5_user,
	    authctxt->pw->pw_name))
		goto err;

	if (client)
		krb5_unparse_name(authctxt->krb5_ctx, authctxt->krb5_user,
		    client);

	ret = 1;
 err:
	if (server)
		krb5_free_principal(authctxt->krb5_ctx, server);
	if (ticket)
		krb5_free_ticket(authctxt->krb5_ctx, ticket);
	if (!ret && reply->length) {
		xfree(reply->data);
		memset(reply, 0, sizeof(*reply));
	}

	if (problem) {
		if (authctxt->krb5_ctx != NULL)
			debug("Kerberos v5 authentication failed: %s",
			    krb5_get_err_text(authctxt->krb5_ctx, problem));
		else
			debug("Kerberos v5 authentication failed: %d",
			    problem);
	}

	return (ret);
}

int
auth_krb5_tgt(Authctxt *authctxt, krb5_data *tgt)
{
	krb5_error_code problem;
	krb5_ccache ccache = NULL;
	char *pname;

	if (authctxt->pw == NULL || authctxt->krb5_user == NULL)
		return (0);

	temporarily_use_uid(authctxt->pw);

	problem = krb5_cc_gen_new(authctxt->krb5_ctx, &krb5_fcc_ops, &ccache);
	if (problem)
		goto fail;

	problem = krb5_cc_initialize(authctxt->krb5_ctx, ccache,
	    authctxt->krb5_user);
	if (problem)
		goto fail;

	problem = krb5_rd_cred2(authctxt->krb5_ctx, authctxt->krb5_auth_ctx,
	    ccache, tgt);
	if (problem)
		goto fail;

	authctxt->krb5_fwd_ccache = ccache;
	ccache = NULL;

	authctxt->krb5_ticket_file = (char *)krb5_cc_get_name(authctxt->krb5_ctx, authctxt->krb5_fwd_ccache);

	problem = krb5_unparse_name(authctxt->krb5_ctx, authctxt->krb5_user,
	    &pname);
	if (problem)
		goto fail;

	debug("Kerberos v5 TGT accepted (%s)", pname);

	restore_uid();

	return (1);

 fail:
	if (problem)
		debug("Kerberos v5 TGT passing failed: %s",
		    krb5_get_err_text(authctxt->krb5_ctx, problem));
	if (ccache)
		krb5_cc_destroy(authctxt->krb5_ctx, ccache);

	restore_uid();

	return (0);
}


int
auth_krb5_password(Authctxt *authctxt, const char *password)
{
	krb5_error_code problem;
	krb5_ccache ccache = NULL;

	if (!authctxt->valid)
		return (0);

	temporarily_use_uid(authctxt->pw);

	problem = krb5_init(authctxt);
	if (problem)
		goto out;

	problem = krb5_parse_name(authctxt->krb5_ctx, authctxt->pw->pw_name,
		    &authctxt->krb5_user);
	if (problem)
		goto out;

	problem = krb5_cc_gen_new(authctxt->krb5_ctx, &krb5_mcc_ops, &ccache);
	if (problem)
		goto out;

	problem = krb5_cc_initialize(authctxt->krb5_ctx, ccache,
		authctxt->krb5_user);
	if (problem)
		goto out;

	restore_uid();

	problem = krb5_verify_user(authctxt->krb5_ctx, authctxt->krb5_user,
	    ccache, password, 1, NULL);

	temporarily_use_uid(authctxt->pw);

	if (problem)
		goto out;

	problem = krb5_cc_gen_new(authctxt->krb5_ctx, &krb5_fcc_ops,
	    &authctxt->krb5_fwd_ccache);
	if (problem)
		goto out;

	problem = krb5_cc_copy_cache(authctxt->krb5_ctx, ccache,
	    authctxt->krb5_fwd_ccache);
	krb5_cc_destroy(authctxt->krb5_ctx, ccache);
	ccache = NULL;
	if (problem)
		goto out;

	authctxt->krb5_ticket_file = (char *)krb5_cc_get_name(authctxt->krb5_ctx,
	    authctxt->krb5_fwd_ccache);

 out:
	restore_uid();

	if (problem) {
		if (ccache)
			krb5_cc_destroy(authctxt->krb5_ctx, ccache);

		if (authctxt->krb5_ctx != NULL)
			debug("Kerberos password authentication failed: %s",
			    krb5_get_err_text(authctxt->krb5_ctx, problem));
		else
			debug("Kerberos password authentication failed: %d",
			    problem);

		krb5_cleanup_proc(authctxt);

		if (options.kerberos_or_local_passwd)
			return (-1);
		else
			return (0);
	}
	return (1);
}

void
krb5_cleanup_proc(Authctxt *authctxt)
{
	debug("krb5_cleanup_proc called");
	if (authctxt->krb5_fwd_ccache) {
		krb5_cc_destroy(authctxt->krb5_ctx, authctxt->krb5_fwd_ccache);
		authctxt->krb5_fwd_ccache = NULL;
	}
	if (authctxt->krb5_user) {
		krb5_free_principal(authctxt->krb5_ctx, authctxt->krb5_user);
		authctxt->krb5_user = NULL;
	}
	if (authctxt->krb5_auth_ctx) {
		krb5_auth_con_free(authctxt->krb5_ctx,
		    authctxt->krb5_auth_ctx);
		authctxt->krb5_auth_ctx = NULL;
	}
	if (authctxt->krb5_ctx) {
		krb5_free_context(authctxt->krb5_ctx);
		authctxt->krb5_ctx = NULL;
	}
}

#endif /* KRB5 */
