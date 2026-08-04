#ifndef TASK_H_
#define TASK_H_

#include <kinclude/ktid.h>
#include <kinclude/clone.h>

extern void free_task(struct itask *p);
extern struct itask *dup_task_struct(struct itask *p);
extern int copy_thread(struct itask *p, struct clone_args *args);
extern struct itask *copy_task(struct ktid *ktid, struct clone_args *args);
extern long kernel_wait3(tid_t utid, int *start_addr, int options);

#endif // TASK_H_
