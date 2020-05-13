#ifndef _SYS_VDSO_H_
#define _SYS_VDSO_H_

#include <sys/time.h>
#include <uvm/uvm_extern.h>

struct vdso {
	struct bintime boottime;
	struct bintime offset;
	uint64_t scale;
	uint64_t mask;
	u_int offset_count;
	u_int lock;

	struct timespec tp_realtime;
	struct timespec tp_uptime;
	struct timespec tp_monotonic;
	struct timespec tp_boottime;
};

extern struct uvm_object *vdso_object;
extern struct vdso *vdso;

//void update_vdso(void);

#endif /* _SYS_VDSO_H_ */
