/*	$OpenBSD$ */
/*
 * Copyright (c) 2000 Poul-Henning Kamp <phk@FreeBSD.org>
 * Copyright (c) 2020 Paul Irofti <paul@irofti.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/atomic.h>
#include <sys/time.h>
#include <sys/timetc.h>

#include <time.h>

/*
 * Return the difference between the timehands' counter value now and what
 * was when we copied it to the timehands' offset_count.
 */
static __inline u_int
tc_delta(struct __timekeep *tk)
{
	return ((tc_get_timecount() - tk->th_offset_count) &
	    tk->tc_counter_mask);
}

static inline void
bintimeaddfrac(const struct bintime *bt, uint64_t x, struct bintime *ct)
{
	ct->sec = bt->sec;
	if (bt->frac > bt->frac + x)
		ct->sec++;
	ct->frac = bt->frac + x;
}

static inline void
BINTIME_TO_TIMESPEC(const struct bintime *bt, struct timespec *ts)
{
	ts->tv_sec = bt->sec;
	ts->tv_nsec = (long)(((uint64_t)1000000000 * (uint32_t)(bt->frac >> 32)) >> 32);
}

static inline void
BINTIME_TO_TIMEVAL(const struct bintime *bt, struct timeval *tv)
{
	tv->tv_sec = bt->sec;
	tv->tv_usec = (long)(((uint64_t)1000000 * (uint32_t)(bt->frac >> 32)) >> 32);
}

static void
binuptime(struct bintime *bt, struct __timekeep *tk)
{
	u_int gen;

	do {
		gen = tk->th_generation;
		membar_consumer();
		*bt = tk->th_offset;
		bintimeaddfrac(bt, tk->th_scale * tc_delta(tk), bt);
		membar_consumer();
	} while (gen == 0 || gen != tk->th_generation);
}

static inline void
bintimeadd(const struct bintime *bt, const struct bintime *ct,
    struct bintime *dt)
{
	dt->sec = bt->sec + ct->sec;
	if (bt->frac > bt->frac + ct->frac)
		dt->sec++;
	dt->frac = bt->frac + ct->frac;
}

static inline void
bintimesub(const struct bintime *bt, const struct bintime *ct,
    struct bintime *dt)
{
	dt->sec = bt->sec - ct->sec;
	if (bt->frac < bt->frac - ct->frac)
		dt->sec--;
	dt->frac = bt->frac - ct->frac;
}

static void
binruntime(struct bintime *bt, struct __timekeep *tk)
{
	u_int gen;

	do {
		gen = tk->th_generation;
		membar_consumer();
		bintimeaddfrac(&tk->th_offset, tk->th_scale * tc_delta(tk), bt);
		bintimesub(bt, &tk->th_naptime, bt);
		membar_consumer();
	} while (gen == 0 || gen != tk->th_generation);
}

static void
bintime(struct bintime *bt, struct __timekeep *tk)
{
	u_int gen;

	do {
		gen = tk->th_generation;
		membar_consumer();
		*bt = tk->th_offset;
		bintimeaddfrac(bt, tk->th_scale * tc_delta(tk), bt);
		bintimeadd(bt, &tk->th_boottime, bt);
		membar_consumer();
	} while (gen == 0 || gen != tk->th_generation);
}

void
_microtime(struct timeval *tvp, struct __timekeep *tk)
{
	struct bintime bt;

	bintime(&bt, tk);
	BINTIME_TO_TIMEVAL(&bt, tvp);
}

void
_nanotime(struct timespec *tsp, struct __timekeep *tk)
{
	struct bintime bt;

	bintime(&bt, tk);
	BINTIME_TO_TIMESPEC(&bt, tsp);
}

void
_nanoruntime(struct timespec *ts, struct __timekeep *tk)
{
	struct bintime bt;

	binruntime(&bt, tk);
	BINTIME_TO_TIMESPEC(&bt, ts);
}


void
_nanouptime(struct timespec *tsp, struct __timekeep *tk)
{
	struct bintime bt;

	binuptime(&bt, tk);
	BINTIME_TO_TIMESPEC(&bt, tsp);
}
