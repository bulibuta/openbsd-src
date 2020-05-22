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

void *elf_aux_timekeep;

/*
 * Needed exec_elf implementation.
 * To be exposed by the kernel later if needed.
 */

#include <sys/exec_elf.h>

typedef struct {
	uint32_t	au_id;				/* 32-bit id */
	uint64_t	au_v;				/* 64-bit value */
} AuxInfo;

enum AuxID {
	AUX_null = 0,
	AUX_ignore = 1,
	AUX_execfd = 2,
	AUX_phdr = 3,			/* &phdr[0] */
	AUX_phent = 4,			/* sizeof(phdr[0]) */
	AUX_phnum = 5,			/* # phdr entries */
	AUX_pagesz = 6,			/* PAGESIZE */
	AUX_base = 7,			/* ld.so base addr */
	AUX_flags = 8,			/* processor flags */
	AUX_entry = 9,			/* a.out entry */
	AUX_sun_uid = 2000,		/* euid */
	AUX_sun_ruid = 2001,		/* ruid */
	AUX_sun_gid = 2002,		/* egid */
	AUX_sun_rgid = 2003,		/* rgid */
	AUX_openbsd_timekeep = 2004,	/* userland clock_gettime */
};


/*
 * Helper functions.
 */

static int
find_timekeep(void)
{
	Elf_Addr *stackp;
	AuxInfo *auxv;
	int found = 0;

	stackp = (Elf_Addr *)environ;
	while (*stackp++) ;		/* pass environment */

	/* look-up timekeep auxv */
	for (auxv = (AuxInfo *)stackp; auxv->au_id != AUX_null; auxv++)
		if (auxv->au_id == AUX_openbsd_timekeep) {
			found = 1;
			break;
		}
	if (found == 0) {
		warnx("%s", "Could not find auxv!");
		return -1;
	}

	elf_aux_timekeep = (void *)auxv->au_v;
	return 0;
}

int
WRAP(clock_gettime)(clockid_t clock_id, struct timespec *tp)
{
	struct __timekeep *timekeep;
	unsigned int seq;

	if (elf_aux_timekeep == NULL && find_timekeep())
		return clock_gettime(clock_id, tp);
	timekeep = elf_aux_timekeep;

	switch (clock_id) {
	case CLOCK_REALTIME:
		do {
			seq = timekeep->seq;
			*tp = timekeep->tp_realtime;
		} while (seq == 0 || seq != timekeep->seq);
		break;
	case CLOCK_UPTIME:
		do {
			seq = timekeep->seq;
			*tp = timekeep->tp_uptime;
		} while (seq == 0 || seq != timekeep->seq);
		break;
	case CLOCK_MONOTONIC:
		do {
			seq = timekeep->seq;
			*tp = timekeep->tp_monotonic;
		} while (seq == 0 || seq != timekeep->seq);
		break;
	case CLOCK_BOOTTIME:
		do {
			seq = timekeep->seq;
			*tp = timekeep->tp_boottime;
		} while (seq == 0 || seq != timekeep->seq);
		break;
	default:
		return clock_gettime(clock_id, tp);
	}
	return 0;
}
DEF_WRAP(clock_gettime);
