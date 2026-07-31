#include <kinclude/sched.h>

static void update_curr_idle(struct rqi *rq);

static void put_prev_task_idle(struct rqi *rq, struct itask *prev, struct itask *next)
{
	(void)prev;
	(void)next;
	update_curr_idle(rq);
}

static void set_next_task_idle(struct rqi *rq, struct itask *next, bool first)
{
	(void)rq;
	(void)next;
	(void)first;
}

struct itask *pick_task_idle(struct rqi *rq)
{
	return rq->idle;
}


static bool dequeue_task_idle(struct rqi *rq, struct itask *p, int flags)
{
	(void)rq;
	(void)p;
	(void)flags;

	return true;
}

static void task_tick_idle(struct rqi *rq, struct itask *curr)
{
	(void)curr;
	update_curr_idle(rq);
}

static void update_curr_idle(struct rqi *rq)
{
	(void)rq;
}


DEFINE_ISCHED_CLASS(idle) = {
	.dequeue_task		= dequeue_task_idle,

	.pick_task		= pick_task_idle,
	.put_prev_task		= put_prev_task_idle,
	.set_next_task		= set_next_task_idle,

	.task_tick		= task_tick_idle,

	.update_curr		= update_curr_idle,
};
