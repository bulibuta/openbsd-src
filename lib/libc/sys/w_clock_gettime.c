/*	$OpenBSD$ */
/*
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

#include <stdlib.h>
#include <time.h>
#include <err.h>

#include <sys/time.h>

int
WRAP(clock_gettime)(clockid_t clock_id, struct timespec *tp)
{
	struct __timekeep *timekeep;
	unsigned int seq;

	if (_timekeep == NULL)
		return clock_gettime(clock_id, tp);
	timekeep = _timekeep;

	switch (clock_id) {
	case CLOCK_REALTIME:
		do {
			seq = timekeep->seq;
			_nanotime(tp, timekeep);
		} while (seq == 0 || seq != timekeep->seq);
		break;
	case CLOCK_UPTIME:
		do {
			seq = timekeep->seq;
			_nanoruntime(tp, timekeep);
		} while (seq == 0 || seq != timekeep->seq);
		break;
	case CLOCK_MONOTONIC:
	case CLOCK_BOOTTIME:
		do {
			seq = timekeep->seq;
			_nanouptime(tp, timekeep);
		} while (seq == 0 || seq != timekeep->seq);
		break;
	default:
		return clock_gettime(clock_id, tp);
	}
	return 0;
}
DEF_WRAP(clock_gettime);
