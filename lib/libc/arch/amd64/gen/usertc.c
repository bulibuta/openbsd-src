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

#include <sys/types.h>
#include <sys/timetc.h>

static uint
rdtsc()
{
	uint32_t hi, lo;
	asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
	return ((uint64_t)lo)|(((uint64_t)hi)<<32);
}

static uint
acpihpet()
{
	return rdtsc(); /* JUST TO COMPILE */
}

static uint (*const get_tc[])(void) =
{
	[TC_TSC] = rdtsc,
	[TC_HPET] = acpihpet,
};

int
tc_get_timecount(struct timekeep *tk, uint *tc)
{
	int tk_user = tk->tk_user;

	if (tc == NULL || tk_user < 1 || tk_user > TC_LAST)
		return -1;

	*tc = (*get_tc[tk_user])();
	return 0;
}
int (*const _tc_get_timecount)(struct timekeep *tk, uint *tc)
	= tc_get_timecount;
