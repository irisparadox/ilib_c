#ifndef ASM_X86_SWITCH_TO_H
#define ASM_X86_SWITCH_TO_H

#include <asm/iptrace.h>

struct itask;

struct itask *__kswitch_to_asm(struct itask *prev,
			       struct itask *next);

struct itask *__kswitch_to(struct itask *prev,
			   struct itask *next);

/*
 * This is the structure point to by thread.sp for an inactive task.
 * The order of the fields must match the code in __kswitch_to_asm().
 */
struct inactive_task_frame {
	unsigned long r15;
	unsigned long r14;
	unsigned long r13;
	unsigned long r12;
	unsigned long bx;

	/*
	 * These two fields must be together. They form a stack frame
	 * header.
	 */
	unsigned long bp;
	unsigned long ret_addr;
};

struct fork_frame {
	struct inactive_task_frame frame;
	struct ipt_regs regs;
};

#define switch_to(prev, next, last)			\
do {							\
	((last) = __kswitch_to_asm((prev), (next)));	\
} while (0)

#endif // ASM_X86_SWITCH_TO_H
