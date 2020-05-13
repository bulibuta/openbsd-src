#ifndef _SYS_TIMEKEEP_H_
#define _SYS_TIMEKEEP_H_

#include <sys/time.h>

struct timekeep {
	struct timespec tp_realtime;
	struct timespec tp_uptime;
	struct timespec tp_monotonic;
	struct timespec tp_boottime;
};

#if defined(_KERNEL)
#include <uvm/uvm_extern.h>

extern struct uvm_object *timekeep_object;
extern struct timekeep *timekeep;
#endif

#endif /* _SYS_TIMEKEEP_H_ */
