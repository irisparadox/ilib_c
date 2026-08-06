#include <string.h>
#include <asm/iptrace.h>
#include <asm/switch_to.h>
#include <kinclude/kfun.h>
#include <kinclude/task.h>
#include <kinclude/sched.h>

static inline unsigned long encode_frame_pointer(struct ipt_regs *regs)
{
	return (unsigned long)regs | 1;
}

/*
 * Construct the child's initial kernel stack.
 *
 * The kernel stack grows downward:
 *
 *           Higher addresses
 *      +-----------------------+  <-- p->stack + KSTACK_SIZE
 *      |       ipt_regs        |
 *      +-----------------------+
 *      |      ret_addr         |  <-- ret_from_fork_asm
 *      |         rbp           |
 *      |         rbx           |
 *      |         r12           |
 *      |         r13           |
 *      |         r14           |
 *      |         r15           |  <-- p->thread.sp
 *      +-----------------------+
 *      |                       |
 *      |    free stack space   |
 *      |                       |
 *      +-----------------------+  <-- p->stack
 *            Lower addresses
 *
 * On the first context switch:
 *
 *   __kswitch_to_asm
 *       -> restores r15-rbp
 *       -> jumps to __kswitch_to()
 *
 *   __kswitch_to()
 *       -> returns
 *
 *   ret
 *       -> ret_from_fork_asm
 */
int copy_thread(struct itask *p, struct clone_args *args)
{
	unsigned long sp = args->stack;
	struct inactive_task_frame *frame;
	struct fork_frame *fork_frame;
	struct ipt_regs *childregs;
	int ret = 0;
	
	childregs = task_pt_regs(p);
	fork_frame = container_of(childregs, struct fork_frame, regs);
	frame = &fork_frame->frame;

	frame->ret_addr = (unsigned long)ret_from_fork_asm;
	frame->bp = encode_frame_pointer(childregs);
	p->thread.sp = (unsigned long)fork_frame;

	/* kernel thread ? */
	if (unlikely(p->flags & TF_KTHREAD)) {
		memset(childregs, 0, sizeof(struct ipt_regs));
		kthread_frame_init(frame, args->fn, args->fn_arg);
		return 0;
	}

	frame->bx = 0;
	*childregs = *current_pt_regs();
	childregs->ax = 0;

	if (sp)
		childregs->sp = sp;

	if (unlikely(args->fn)) {
		childregs->sp = 0;
		childregs->ip = 0;
		kthread_frame_init(frame, args->fn, args->fn_arg);
		return 0;
	}
	
	return ret;
}

void ret_from_fork(struct itask *prev, struct ipt_regs *regs,
		   int (*fn)(void *), void *fn_arg)
{
	schedule_tail(prev);

	if (unlikely(fn)) {
		fn(fn_arg);
		
	}
}
