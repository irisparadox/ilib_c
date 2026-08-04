#ifndef CLONE_H_
#define CLONE_H_

#include <kinclude/ktypes.h>

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
	u64	stack;
	u64	stack_size;
};

#endif // CLONE_H_
