#ifndef KTID_H_
#define KTID_H_

#include <pthread.h>
#include <deftypei.h>
#include <ilisti.h>
#include <kinclude/ktypes.h>

#define tid_t __ikernel_tid
#define TID_MAX 32768

struct ktid {
	tid_t          	 id;
	struct itask   	*thread;
	struct ilinode 	 ktid_link;
};

struct ktid_alloc {
	struct ktid     	slots[TID_MAX];
	struct ilinode  	ktid_head;
	tid_t           	next_tid;
	pthread_mutex_t 	lock;
};

extern struct itask *tid_task(struct ktid_alloc *al, int tid);
extern struct itask *ktid_get_task(struct ktid *ktid);
extern struct ktid  *tid_get_ktid(struct ktid_alloc *al, int tid);

static inline bool ktid_has_task(struct ktid *ktid)
{
	return ktid->thread != NULL;
}

extern void attach_ktid(struct itask *task);
extern void detach_ktid(struct itask *task);
extern void change_ktid(struct itask *task, struct ktid *ktid);


extern void ktid_init(struct ktid_alloc *al);
extern struct ktid *ktid_alloc(struct ktid_alloc *al);
extern void  free_tid(struct ktid_alloc *al, struct ktid *ktid);

#endif // KTID_H_
