/*	$OpenBSD$ */
/*
 * Copyright (c) 2020 Robert Nagy <robert@openbsd.org>
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

#include <sys/time.h>

int
WRAP(gettimeofday)(struct timeval *tp, struct timezone *tzp)
{
	struct __timekeep *timekeep = _timekeep;
	static struct timezone zerotz = { 0, 0 };
	unsigned int seq;

	if (timekeep == NULL)
		return gettimeofday(tp, tzp);

	if (tp) {
		do {
			seq = timekeep->seq;
			*tp = timekeep->tp_microtime;
		} while (seq == 0 || seq != timekeep->seq);
	}

	if (tzp)
		tzp = &zerotz;

	return 0;
}
DEF_WRAP(gettimeofday);
