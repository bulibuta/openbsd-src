/*	$OpenBSD$	*/
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

#ifndef _LIBC_SYS_TIMETC_H_
#define _LIBC_SYS_TIMETC_H_

#define _LIBC
#include <sys/types.h>
#include <sys/time.h>

#include_next <sys/timetc.h>

__BEGIN_HIDDEN_DECLS
extern void *_timekeep;

extern uint64_t (*const tc_get_timecount)(void);
uint64_t tc_get_timecount_md(void);

void _microtime(struct timeval *tvp, struct __timekeep *tk);
void _nanotime(struct timespec *tsp, struct __timekeep *tk);
void _nanoruntime(struct timespec *ts, struct __timekeep *tk);
void _nanouptime(struct timespec *tsp, struct __timekeep *tk);
__END_HIDDEN_DECLS

#endif /* !_LIBC_SYS_TIMETC_H_ */
