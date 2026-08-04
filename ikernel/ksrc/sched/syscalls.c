#include <kinclude/sched.h>
#include <kinclude/syscall.h>

static void do_sched_yield(void)
{
	struct rqi *rq;

	rq = this_rq_lock();
	schedstat_inc(rq->yld_count);

	rq->curr->sched_class->yield_task(rq);

	rq_unlock(rq);

	schedule();
}

SYSCALL_DEFINE0(sched_yield)
{
	do_sched_yield();
	return 0;
}
