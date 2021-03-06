/*	$NetBSD: pthread_mutex2.c,v 1.9 2007/09/21 21:28:11 ad Exp $	*/

/*-
 * Copyright (c) 2001, 2003, 2006, 2007 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Nathan J. Williams, by Jason R. Thorpe, and by Andrew Doran.
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
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
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
__RCSID("$NetBSD: pthread_mutex2.c,v 1.9 2007/09/21 21:28:11 ad Exp $");

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "pthread.h"
#include "pthread_int.h"

#ifdef	PTHREAD__HAVE_ATOMIC

/*
 * Note that it's important to use the address of ptm_waiters as
 * the list head in order for the hint arguments to _lwp_park /
 * _lwp_unpark_all to match.
 */
#define	pt_nextwaiter			pt_sleep.ptqe_next
#define	ptm_waiters			ptm_blocked.ptqh_first
#define	ptm_errorcheck			ptm_blocked.ptqh_last

#define	MUTEX_WAITERS_BIT		(0x01UL)
#define	MUTEX_RECURSIVE_BIT		(0x02UL)
#define	MUTEX_THREAD			(-16L)

#define	MUTEX_DEFERRED(x)		(*(char *)&(x)->ptm_lock)

#define	MUTEX_HAS_WAITERS(x)		((uintptr_t)(x) & MUTEX_WAITERS_BIT)
#define	MUTEX_RECURSIVE(x)		((uintptr_t)(x) & MUTEX_RECURSIVE_BIT)
#define	MUTEX_OWNER(x)			((uintptr_t)(x) & MUTEX_THREAD)
#define	MUTEX_GET_RECURSE(ptm)		((intptr_t)(ptm)->ptm_private)
#define	MUTEX_SET_RECURSE(ptm, delta)	\
    ((ptm)->ptm_private = (void *)((intptr_t)(ptm)->ptm_private + delta))

#if __GNUC_PREREQ__(3, 0)
#define	NOINLINE		__attribute ((noinline))
#else
#define	NOINLINE		/* nothing */
#endif

static void	pthread__mutex_wakeup(pthread_t, pthread_mutex_t *);
static int	pthread__mutex_lock_slow(pthread_mutex_t *);
static int	pthread__mutex_unlock_slow(pthread_mutex_t *);
static void	pthread__mutex_pause(void);

__strong_alias(__libc_mutex_init,pthread_mutex_init)
__strong_alias(__libc_mutex_lock,pthread_mutex_lock)
__strong_alias(__libc_mutex_trylock,pthread_mutex_trylock)
__strong_alias(__libc_mutex_unlock,pthread_mutex_unlock)
__strong_alias(__libc_mutex_destroy,pthread_mutex_destroy)

__strong_alias(__libc_mutexattr_init,pthread_mutexattr_init)
__strong_alias(__libc_mutexattr_destroy,pthread_mutexattr_destroy)
__strong_alias(__libc_mutexattr_settype,pthread_mutexattr_settype)

__strong_alias(__libc_thr_once,pthread_once)

static inline int
mutex_cas(volatile void *ptr, void **old, void *new)
{

	return pthread__atomic_cas_ptr(ptr, old, new);
}
	
int
pthread_mutex_init(pthread_mutex_t *ptm, const pthread_mutexattr_t *attr)
{
	intptr_t type;

	if (attr == NULL)
		type = PTHREAD_MUTEX_NORMAL;
	else
		type = (intptr_t)attr->ptma_private;

	switch (type) {
	case PTHREAD_MUTEX_ERRORCHECK:
		ptm->ptm_errorcheck = (void *)1;
		ptm->ptm_owner = NULL;
		break;
	case PTHREAD_MUTEX_RECURSIVE:
		ptm->ptm_errorcheck = NULL;
		ptm->ptm_owner = (void *)MUTEX_RECURSIVE_BIT;
		break;
	default:
		ptm->ptm_errorcheck = NULL;
		ptm->ptm_owner = NULL;
		break;
	}

	ptm->ptm_magic = _PT_MUTEX_MAGIC;
	ptm->ptm_waiters = NULL;
	ptm->ptm_private = NULL;
	MUTEX_DEFERRED(ptm) = 0;

	return 0;
}


int
pthread_mutex_destroy(pthread_mutex_t *ptm)
{

	pthread__error(EINVAL, "Invalid mutex",
	    ptm->ptm_magic == _PT_MUTEX_MAGIC);
	pthread__error(EBUSY, "Destroying locked mutex",
	    MUTEX_OWNER(ptm->ptm_owner) == 0);

	ptm->ptm_magic = _PT_MUTEX_DEAD;
	return 0;
}


/*
 * Note regarding memory visibility: Pthreads has rules about memory
 * visibility and mutexes. Very roughly: Memory a thread can see when
 * it unlocks a mutex can be seen by another thread that locks the
 * same mutex.
 * 
 * A memory barrier after a lock and before an unlock will provide
 * this behavior. This code relies on mutex_cas() to issue a barrier
 * after obtaining a lock, and on pthread__simple_unlock() to issue
 * a barrier before releasing a lock.
 */

int
pthread_mutex_lock(pthread_mutex_t *ptm)
{
	void *owner;
	pthread_t self;

	owner = NULL;
	self = pthread__self();

	if (__predict_true(mutex_cas(&ptm->ptm_owner, &owner, self)))
		return 0;

	return pthread__mutex_lock_slow(ptm);
}

/* We want function call overhead. */
NOINLINE static void
pthread__mutex_pause(void)
{

	pthread__smt_pause();
}

NOINLINE static int
pthread__mutex_lock_slow(pthread_mutex_t *ptm)
{
	void *waiters, *new, *owner;
	pthread_t self;
	int count;

	pthread__error(EINVAL, "Invalid mutex",
	    ptm->ptm_magic == _PT_MUTEX_MAGIC);

	owner = ptm->ptm_owner;
	self = pthread__self();

	/* Recursive or errorcheck? */
	if (MUTEX_OWNER(owner) == (uintptr_t)self) {
		if (MUTEX_RECURSIVE(owner)) {
			if (MUTEX_GET_RECURSE(ptm) == INT_MAX)
				return EAGAIN;
			MUTEX_SET_RECURSE(ptm, +1);
			return 0;
		}
		if (ptm->ptm_errorcheck)
			return EDEADLK;
	}

	/* Spin for a while. */
	count = pthread__nspins;
	while (MUTEX_OWNER(owner) == 0 && --count > 0) {
		pthread__mutex_pause();
		owner = ptm->ptm_owner;
	}

	for (;; owner = ptm->ptm_owner) {
		/* If it has become free, try to acquire it again. */
		while (MUTEX_OWNER(owner) == 0) {
			new = (void *)((uintptr_t)self | (uintptr_t)owner);
			if (mutex_cas(&ptm->ptm_owner, &owner, new))
				return 0;
		}

		/*
		 * Nope, still held.  Add thread to the list of waiters.
		 * Issue a memory barrier to ensure sleeponq/nextwaiter
		 * are visible before we enter the waiters list.
		 */
		self->pt_sleeponq = 1;
		for (waiters = ptm->ptm_waiters;;) {
			self->pt_nextwaiter = waiters;
			pthread__membar_producer();
			if (mutex_cas(&ptm->ptm_waiters, &waiters, self))
			    	break;
		}

		/*
		 * Set the waiters bit and block.
		 *
		 * Note that the mutex can become unlocked before we set
		 * the waiters bit.  If that happens it's not safe to sleep
		 * as we may never be awoken: we must remove the current
		 * thread from the waiters list and try again.
		 *
		 * Because we are doing this atomically, we can't remove
		 * one waiter: we must remove all waiters and awken them,
		 * then sleep in _lwp_park() until we have been awoken. 
		 *
		 * Issue a memory barrier to ensure that we are reading
		 * the value of ptm_owner/pt_sleeponq after we have entered
		 * the waiters list (the CAS itself must be atomic).
		 */
		pthread__membar_consumer();
		for (owner = ptm->ptm_owner;;) {
			if (MUTEX_HAS_WAITERS(owner))
				break;
			if (MUTEX_OWNER(owner) == 0) {
				pthread__mutex_wakeup(self, ptm);
				break;
			}
			new = (void *)((uintptr_t)owner | MUTEX_WAITERS_BIT);
			if (mutex_cas(&ptm->ptm_owner, &owner, new))
				break;
		}

		/*
		 * We may be awoken by this thread, or some other thread.
		 * The key requirement is that we must not proceed until
		 * told that we are no longer waiting (via pt_sleeponq
		 * being set to zero),  Otherwise it is unsafe to re-enter
		 * the thread onto the waiters list.
		 */
		while (self->pt_sleeponq) {
			(void)_lwp_park(NULL, 0, &ptm->ptm_waiters, NULL);
		}
	}
}

int
pthread_mutex_trylock(pthread_mutex_t *ptm)
{
	pthread_t self;
	void *value;

	self = pthread__self();
	value = NULL;

	if (mutex_cas(&ptm->ptm_owner, &value, self))
		return 0;

	if (MUTEX_OWNER(value) == (uintptr_t)self && MUTEX_RECURSIVE(value)) {
		if (MUTEX_GET_RECURSE(ptm) == INT_MAX)
			return EAGAIN;
		MUTEX_SET_RECURSE(ptm, +1);
		return 0;
	}

	return EBUSY;
}

NOINLINE int
pthread__mutex_catchup(pthread_mutex_t *ptm)
{
	pthread_t self;

	self = pthread__self();

	if (self->pt_nwaiters == 1) {
		/*
		 * If the calling thread is about to block, defer
		 * unparking the target until _lwp_park() is called.
		 */
		if (self->pt_willpark && self->pt_unpark == 0) {
			self->pt_unpark = self->pt_waiters[0];
			self->pt_unparkhint = &ptm->ptm_waiters;
		} else {
			(void)_lwp_unpark(self->pt_waiters[0],
			    &ptm->ptm_waiters);
		}
	} else {
		(void)_lwp_unpark_all(self->pt_waiters, self->pt_nwaiters,
		    &ptm->ptm_waiters);
	}
	self->pt_nwaiters = 0;

	return 0;
}

int
pthread_mutex_unlock(pthread_mutex_t *ptm)
{
	void *owner;
	pthread_t self;
	char deferred;

	self = pthread__self();
	owner = self;
	deferred = MUTEX_DEFERRED(ptm);
	MUTEX_DEFERRED(ptm) = 0;

	if (__predict_false(!mutex_cas(&ptm->ptm_owner, &owner, NULL)))
		return pthread__mutex_unlock_slow(ptm);

	/*
	 * There were no waiters, but we may have deferred waking
	 * other threads until mutex unlock - we must wake them now.
	 */
	if (deferred)
		return pthread__mutex_catchup(ptm);

	return 0;
}

NOINLINE static int
pthread__mutex_unlock_slow(pthread_mutex_t *ptm)
{
	pthread_t self, owner, new;
	int weown, error;

	pthread__error(EINVAL, "Invalid mutex",
	    ptm->ptm_magic == _PT_MUTEX_MAGIC);

	self = pthread_self();
	owner = ptm->ptm_owner;
	weown = (MUTEX_OWNER(owner) == (uintptr_t)self);
	error = 0;

	if (ptm->ptm_errorcheck) {
		if (!weown) {
			error = EPERM;
			new = owner;
		} else {
			new = NULL;
		}
	} else if (MUTEX_RECURSIVE(owner)) {
		if (!weown) {
			error = EPERM;
			new = owner;
		} else if (MUTEX_GET_RECURSE(ptm) != 0) {
			MUTEX_SET_RECURSE(ptm, -1);
			new = owner;
		} else {
			new = (pthread_t)MUTEX_RECURSIVE_BIT;
		}
	} else {
		pthread__error(EPERM,
		    "Unlocking unlocked mutex", (owner != NULL));
		pthread__error(EPERM,
		    "Unlocking mutex owned by another thread", weown);
		new = NULL;
	}

	/*
	 * Release the mutex.  If there appear to be waiters, then
	 * wake them up.
	 */
	if (new != owner) {
		owner = pthread__atomic_swap_ptr(&ptm->ptm_owner, new);
		if (MUTEX_HAS_WAITERS(owner) != 0) {
			pthread__mutex_wakeup(self, ptm);
			return 0;
		}
	}

	/*
	 * There were no waiters, but we may have deferred waking
	 * other threads until mutex unlock - we must wake them now.
	 */
	if (self->pt_nwaiters != 0)
		return pthread__mutex_catchup(ptm);
	return error;
}

static void
pthread__mutex_wakeup(pthread_t self, pthread_mutex_t *ptm)
{
	pthread_t thread, next;
	ssize_t n, rv;

	/*
	 * Take ownership of the current set of waiters.  No
	 * need for a memory barrier following this, all loads
	 * are dependent upon 'thread'.
	 */
	thread = pthread__atomic_swap_ptr(&ptm->ptm_waiters, NULL);

	for (;; n = 0) {
		/*
		 * Pull waiters from the queue and add to our list.
		 * Use a memory barrier to ensure that we safely
		 * read the value of pt_nextwaiter before 'thread'
		 * sees pt_sleeponq being cleared.
		 */
		for (n = self->pt_nwaiters, self->pt_nwaiters = 0;
		    n < pthread__unpark_max && thread != NULL;
		    thread = next) {
		    	next = thread->pt_nextwaiter;
			self->pt_waiters[n++] = thread->pt_lid;
			pthread__membar_full();
			thread->pt_sleeponq = 0;
			/* No longer safe to touch 'thread' */
		}

		switch (n) {
		case 0:
			return;
		case 1:
			/*
			 * If the calling thread is about to block,
			 * defer unparking the target until _lwp_park()
			 * is called.
			 */
			if (self->pt_willpark && self->pt_unpark == 0) {
				self->pt_unpark = self->pt_waiters[0];
				self->pt_unparkhint = &ptm->ptm_waiters;
				return;
			}
			rv = (ssize_t)_lwp_unpark(self->pt_waiters[0],
			    &ptm->ptm_waiters);
			if (rv != 0 && errno != EALREADY && errno != EINTR) {
				pthread__errorfunc(__FILE__, __LINE__,
				    __func__, "_lwp_unpark failed");
			}
			return;
		default:
			rv = _lwp_unpark_all(self->pt_waiters, (size_t)n,
			    &ptm->ptm_waiters);
			if (rv != 0 && errno != EINTR) {
				pthread__errorfunc(__FILE__, __LINE__,
				    __func__, "_lwp_unpark_all failed");
			}
			break;
		}
	}
}
int
pthread_mutexattr_init(pthread_mutexattr_t *attr)
{

	attr->ptma_magic = _PT_MUTEXATTR_MAGIC;
	attr->ptma_private = (void *)PTHREAD_MUTEX_DEFAULT;
	return 0;
}

int
pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{

	pthread__error(EINVAL, "Invalid mutex attribute",
	    attr->ptma_magic == _PT_MUTEXATTR_MAGIC);

	return 0;
}


int
pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *typep)
{

	pthread__error(EINVAL, "Invalid mutex attribute",
	    attr->ptma_magic == _PT_MUTEXATTR_MAGIC);

	*typep = (int)(intptr_t)attr->ptma_private;
	return 0;
}


int
pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{

	pthread__error(EINVAL, "Invalid mutex attribute",
	    attr->ptma_magic == _PT_MUTEXATTR_MAGIC);

	switch (type) {
	case PTHREAD_MUTEX_NORMAL:
	case PTHREAD_MUTEX_ERRORCHECK:
	case PTHREAD_MUTEX_RECURSIVE:
		attr->ptma_private = (void *)(intptr_t)type;
		return 0;
	default:
		return EINVAL;
	}
}


static void
once_cleanup(void *closure)
{

       pthread_mutex_unlock((pthread_mutex_t *)closure);
}


int
pthread_once(pthread_once_t *once_control, void (*routine)(void))
{

	if (once_control->pto_done == 0) {
		pthread_mutex_lock(&once_control->pto_mutex);
		pthread_cleanup_push(&once_cleanup, &once_control->pto_mutex);
		if (once_control->pto_done == 0) {
			routine();
			once_control->pto_done = 1;
		}
		pthread_cleanup_pop(1);
	}

	return 0;
}

int
pthread__mutex_deferwake(pthread_t thread, pthread_mutex_t *ptm)
{

	if (MUTEX_OWNER(ptm->ptm_owner) != (uintptr_t)thread)
		return 0;
	MUTEX_DEFERRED(ptm) = 1;
	return 1;	
}

#endif	/* PTHREAD__HAVE_ATOMIC */
