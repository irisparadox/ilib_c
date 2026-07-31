#include <kinclude/sched.h>

static void __schedule(int sched_mode)
{
	struct itask *prev, *next;
	bool preempt = sched_mode > SM_NONE;
	bool is_switch;
	unsigned long *switch_count;
	unsigned long prev_state;
	struct rqi *rq;

	rq = cpu_rq();
	prev = rq->curr;

	pthread_mutex_lock(&rq->__lock);

	switch_count = &prev->nivcsw;

	preempt = sched_mode = SM_PREEMPT;

	prev_state = IREAD_ONCE(unsigned int, prev->__state);
	if (sched_mode == SM_IDLE) {
		if (!rq->nr_running) {
			next = prev;
			rq->next_class = &idle_sched_class;
			goto picked;
		}
	} else if (!preempt && prev_state) {
		prev->sched_class->dequeue_task(rq, prev, 0);
		switch_count = &prev->nvcsw;
	}

	next = pick_next_task(rq);
	rq->next_class = next->sched_class;
 picked:
	clear_tsk_need_resched(prev);
	is_switch = prev != next;
	if (likely(is_switch)) {
		rq->nr_switches++;
		++(*switch_count);

		rq = contex_switch(rq, prev, next);
	} else {
		pthread_mutex_unlock(&rq->__lock);
	}
}

static __ialways_inline void __schedule_loop(int sched_mode)
{
	do {
		__schedule(sched_mode);
	} while (need_resched());
}

void schedule(void)
{
	__schedule_loop(SM_NONE);
}
