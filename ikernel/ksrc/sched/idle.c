#include <stdlib.h>
#include <ifutex.h>
#include <kinclude/sched.h>
#include <kinclude/vcpu.h>
#include <kinclude/kfun.h>
#include <kinclude/init.h>

void cpu_idle_prepare(void)
{
	struct rqi   *rq  = cpu_rq();
	struct ikern *ker = ikern_self();

	rq->idle = calloc(1, sizeof(struct itask));
	if (unlikely(rq->idle == NULL))
		BUG();
	/* we can't go on with initialization if idle can't be allocated */

	struct itask *idle = rq->idle;
	idle->tid  = ktid_alloc();
	attach_ktid(idle);
	idle->prio = MAX_PRIO - 1;

	idle->sched_class = &idle_sched_class;
	idle->__state     = TASK_RUNNING;
	idle->flags       = 0;

	ilisti_init(&idle->children);
	ilisti_init(&idle->sibling);
	ilisti_init(&idle->wait_chldexit.head);
	ilisti_init(&idle->run_list);
	ilisti_init(&idle->wait_entry.entry);

	pthread_mutex_init(&idle->sleep_lock, NULL);
	pthread_cond_init(&idle->sleep_signal, NULL);

	rq->curr    = idle;
	idle->on_rq = TASK_ON_RQ_QUEUED;
}

static void do_idle(void)
{
	struct ivcpu *cpu = cpu_self();
	struct rqi   *rq  = cpu_rq();

	if (cpu_is_offline(cpu)) {
		idle_play_dead();
	}

	while (need_resched()) {
		futex_wait_zero(&rq->idle->flags);
	}

	schedule_idle();
}

void cpu_startup_entry(void)
{
	while (1)
		do_idle();
}

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
