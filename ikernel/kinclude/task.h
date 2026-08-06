#ifndef TASK_H_
#define TASK_H_

#include <kinclude/ktid.h>

/*
 * struct clone_args - arguments for the clone3 syscall
 * stack:	Specify the location of the stack for the
 * 		child task.
 * 		Note, stack is expected to point to the
 * 		lowest address. The stack direciton will be
 * 		determined by the kernel and set up
 * 		appropiately based on stack_size.
 * stack_size:	The size of the stack for the child task.
 */
struct clone_args {
	const char *name;
	u64	stack;
	u64	stack_size;
	int (*fn)(void *);
	void *fn_arg;
};

extern void free_task(struct itask *p);
extern struct itask *dup_task_struct(struct itask *p);
extern int copy_thread(struct itask *p, struct clone_args *args);
extern struct itask *copy_task(struct ktid *ktid, struct clone_args *args);
extern tid_t kernel_clone(struct clone_args *args);
extern tid_t kernel_thread(int (*fn)(void *), void *arg, const char *name);
extern tid_t user_mode_thread(int (*fn)(void *), void *arg);
extern long kernel_wait3(tid_t utid, int *start_addr, int options);

#endif // TASK_H_
