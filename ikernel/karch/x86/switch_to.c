#include <kinclude/sched.h>
#include <asm/switch_to.h>
#include <asm/processor.h>

struct itask *__kswitch_to(struct itask *prev_p, struct itask *next_p)
{
	struct ithread_struct *prev = &prev_p->thread;
	struct ithread_struct *next = &next_p->thread;

	current = next_p;
	current_kernel_rsp = (void *)task_top_of_stack(next_p);

	return prev_p;
}
