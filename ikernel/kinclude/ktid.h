#ifndef KTID_H_
#define KTID_H_

#include <pthread.h>
#include <deftypei.h>
#include <kinclude/ktypes.h>

#define tid_t __ikernel_tid
#define TID_MAX 32768

typedef struct ktid ktid;

struct ktid {
	tid_t id;
};

struct ktid_alloc {
	pthread_mutex_t lock;
	DECLARE_BITMAP(bitmap, TID_MAX);
};

extern tid_t tid_alloc(struct ktid_alloc *al);
extern void  free_tid(struct ktid_alloc *al, tid_t tid);

#endif // KTID_H_
