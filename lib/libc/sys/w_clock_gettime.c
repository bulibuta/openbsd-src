/*	$OpenBSD$ */
/*
 * Copyright (c) 2019 Nicolas Manichon <nicolas.manichon@epita.fr>
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
#include <errno.h>
#include <err.h>

#include <elf.h>


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
	AUX_openbsd_vdso = 4242,
};


/*
 * Needed microtime(9) implementation.
 * To be exposed by the kernel later if needed.
 */

#include <sys/time.h>

struct bintime {
	time_t sec;
	uint64_t frac;
};

static __inline void bintimeaddfrac(const struct bintime *bt, uint64_t x,
				    struct bintime *ct) {
	ct->sec = bt->sec;
	if (bt->frac > bt->frac + x) ct->sec++;
	ct->frac = bt->frac + x;
}

static __inline void bintimeadd(const struct bintime *bt,
				const struct bintime *ct, struct bintime *dt) {
	dt->sec = bt->sec + ct->sec;
	if (bt->frac > bt->frac + ct->frac) dt->sec++;
	dt->frac = bt->frac + ct->frac;
}

static inline void
BINTIME_TO_TIMESPEC(const struct bintime *bt, struct timespec *ts)
{
	ts->tv_sec = bt->sec;
	ts->tv_nsec = (long)(((uint64_t)1000000000 * (uint32_t)(bt->frac >> 32)) >> 32);
}

static __inline uint64_t rdtsc(void) {
	uint32_t hi, lo;

	__asm volatile("rdtsc" : "=d"(hi), "=a"(lo));
	return (((uint64_t)hi << 32) | (uint64_t)lo);
}

static __inline u_int tc_delta(u_int count, uint64_t mask, u_int offset) {
	return (count - offset) & mask;
}

#include <sys/vdso.h>

void *elf_aux_vdso;

/*
 * Helper functions.
 */

static int
find_vdso(void)
{
	Elf_Addr *stackp;
	AuxInfo *auxv;
	int found = 0;

	stackp = (Elf_Addr *)environ;
	while (*stackp++) ;		/* pass environment */

	/* look-up vdso auxv */
	for (auxv = (AuxInfo *)stackp; auxv->au_id != AUX_null; auxv++)
		if (auxv->au_id == AUX_openbsd_vdso) {
			found = 1;
			break;
		}
	if (found == 0) {
		warnx("%s", "Could not find auxv!");
		return -1;
	}

	elf_aux_vdso = (void *)auxv->au_v;
	return 0;
}

#if 0
static int
gettime(struct vdso *vdso, struct timespec *tp)
{
	while (1) {
		uint32_t v = vdso->lock;
		if (v % 2 == 0) {
			struct bintime bt = vdso->offset;
			struct bintime boottime = vdso->boottime;

			u_int delta = tc_delta(rdtsc(), vdso->mask,
			    vdso->offset_count);
			bintimeadd(&bt, &boottime, &bt);
			bintimeaddfrac(&bt, vdso->scale * delta, &bt);
			BINTIME_TO_TIMESPEC(&bt, tp);

			if (vdso->lock != v)
				continue;
			return 0;
		}
	}
	return -1;
}
#endif

int
WRAP(clock_gettime)(clockid_t clock_id, struct timespec *tp)
{
	struct vdso *vdso;

	if (elf_aux_vdso == NULL && find_vdso())
		return clock_gettime(clock_id, tp);
	vdso = elf_aux_vdso;

	switch (clock_id) {
	case CLOCK_REALTIME:
		*tp = vdso->tp_realtime;
		break;
	case CLOCK_UPTIME:
		*tp = vdso->tp_uptime;
		break;
	case CLOCK_MONOTONIC:
		*tp = vdso->tp_monotonic;
		break;
	case CLOCK_BOOTTIME:
		*tp = vdso->tp_boottime;
		break;
	default:
		return clock_gettime(clock_id, tp);
	}
	return 0;
}
DEF_WRAP(clock_gettime);
