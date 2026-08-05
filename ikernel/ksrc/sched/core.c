#include <kinclude/sched.h>
#include <kinclude/kfun.h>
#include <asm/switch_to.h>
#include <kinclude/init.h>

static __ialways_inline bool is_idle_task(const struct itask *p)
{
	return !!(p->flags & TF_IDLE);
}

static __always_inline void set_task_need_resched(struct itask *p)
{
	__katomic_long_or(TIF_NEED_RESCHED, &p->flags);
}

/*
 * __resched_curr - mark the current task for rescheduling.
 *
 * Sets the appropriate reschedule flag for the task currently executing on
 * @rq. If the current task is the idle task, an immediate reschedule is
 * always requested since there is no reason to defer runnable work.
 *
 * This function only marks the task as needing rescheduling; the scheduler
 * will observe the flag at the next rescheduling point.
 */
static void __resched_curr(struct rqi *rq, int tif)
{
	struct itask *curr = rq->curr;

	/*
	 * Always preempt the idle task.
	 */
	if (is_idle_task(curr))
		tif = TIF_NEED_RESCHED;

	if (tif == TIF_NEED_RESCHED)
		set_task_need_resched(curr);
}

int default_wake_function(struct iwait_queue_entry *wq_entry, unsigned int mode, int flags, void *key)
{
	return try_to_wake_up(wq_entry->private, mode, flags);
}

static __ialways_inline
int __task_state_match(struct itask *p, unsigned int state)
{
	if (IREAD_ONCE(unsigned int, p->__state) & state)
		return 1;

	return 0;
}

static __ialways_inline
int ttwu_state_match(struct itask *p, unsigned int state, int *success)
{
	int match;

	*success = !!(match = __task_state_match(p, state));

	return match > 0;
}

/*
 * Mark the task runnable.
 */
static inline void ttwu_do_wakeup(struct itask *p)
{
	IWRITE_ONCE(unsigned int, p->__state, TASK_RUNNING);
}

static void ttwu_do_activate(struct rqi *rq, struct itask *p, int flags)
{
	activate_task(rq, p, flags);
	wakeup_preempt(rq, p, flags);

	ttwu_do_wakeup(p);

	if (p->sched_class->task_woken) {
		rq_unlock(rq);
		p->sched_class->task_woken(rq, p);
		rq_lock(rq);
	}
}

static void ttwu_queue(struct itask *p, int flags)
{
	struct rqi *rq = cpu_rq();

	rq_lock(rq);
	ttwu_do_activate(rq, p, flags);
	rq_unlock(rq);
}

/*
 * Fast wakeup path for tasks that are still queued on a runqueue.
 *
 * A task may have already transitioned into a sleeping state while still
 * remaining runnable because it has not yet been descheduled. In that case
 * there is no need to enqueue it again; simply restore TASK_RUNNING while
 * holding the runqueue lock.
 *
 * Taking @p's runqueue lock serializes against the scheduler. If the task is
 * no longer queued, it has already been descheduled and the caller must fall
 * back to the full wakeup path, which transitions the task through
 * TASK_WAKING and re-enqueues it.
 *
 * Return: %true if the wakeup completed,
 *         %false if a full wakeup is required.
 */
static int ttwu_runnable(struct itask *p, int flags)
{
	struct rqi *rq = task_rq(p);
	rq_lock(rq);

	if (!task_on_rq_queued(p)) {
		rq_unlock(rq);
		return 0;
	}	

	ttwu_do_wakeup(p);
	return 1;
}

void enqueue_task(struct rqi *rq, struct itask *p, int flags)
{
	p->sched_class->enqueue_task(rq, p, flags);
}

inline bool dequeue_task(struct rqi *rq, struct itask *p, int flags)
{
	return p->sched_class->dequeue_task(rq, p, flags);
}

void activate_task(struct rqi *rq, struct itask *p, int flags)
{
	enqueue_task(rq, p, flags);

	IWRITE_ONCE(u8, p->on_rq, TASK_ON_RQ_QUEUED);
}

void wakeup_preempt(struct rqi *rq, struct itask *p, int flags)
{
	if (p->sched_class == rq->next_class) {
		rq->next_class->wakeup_preempt(rq, p, flags);
	} else if (sched_class_above(p->sched_class, rq->next_class)) {
		resched_curr(rq);
		rq->next_class = p->sched_class;
	}
}

/**
 * try_to_wake_up - attempt to wake a sleeping task
 * @p: task to wake
 * @state: mask of task states eligible for wakeup
 * @flags: wakeup modifier flags (WF_*)
 *
 * Conceptually does:
 *
 *   If (@state & @p->__state)
 *       @p->__state = TASK_RUNNING.
 *
 * If the task is not currently runnable, transitions it through
 * TASK_WAKING and queues it on an appropriate runqueue.
 *
 * Wakeups of the current task are handled as a special case and do not
 * require taking the sleep lock.
 *
 * Uses @p->sleep_lock to serialize concurrent wakeups and races against
 * blocking paths. The task state and runqueue membership are revalidated
 * while holding the lock before queueing the task.
 *
 * Return: %true if a wakeup was performed, %false otherwise.
 */
int try_to_wake_up(struct itask *p, unsigned int state, int flags)
{
	int success = 0;

	if (p == current) {
		if (!ttwu_state_match(p, state, &success))
			goto out;

		ttwu_do_wakeup(p);
		goto out;
	}

	scoped_mutex(&p->sleep_lock) {
		if (!ttwu_state_match(p, state, &success))
			break;

		if (IREAD_ONCE(u8, p->on_rq) && ttwu_runnable(p, flags))
			break;

		IWRITE_ONCE(unsigned int, p->__state, TASK_WAKING);

		ttwu_queue(p, flags);
	}

 out:
	return success;
}

void resched_curr(struct rqi *rq)
{
	__resched_curr(rq, TIF_NEED_RESCHED);
}

/*
 * context_switch - switch to the next task's memory map and register state.
 */
static __ialways_inline struct rqi *context_switch(struct rqi *rq, struct itask *prev, struct itask *next)
{
	switch_to(prev, next, prev);
	barrier();

	rq = cpu_rq();
	pthread_mutex_unlock(&rq->__lock);

	return rq;
}

static void put_prev_set_next_task(struct rqi *rq, struct itask *prev, struct itask *next)
{
	if (next == prev)
		return;

	prev->sched_class->put_prev_task(rq, prev, next);
	next->sched_class->set_next_task(rq, next, true);
}

static struct itask *pick_next_task(struct rqi *rq)
{
	const struct isched_class *class;
	struct itask *p;
	u64 i;

	for (i = 0; i < IARRAY_SIZE(sched_classes); ++i) {
		class = sched_classes[i];

		p = class->pick_task(rq);

		if (likely(p)) {
			put_prev_set_next_task(rq, rq->curr, p);
			return p;
		}
	}

	BUG(); /* idle class should have runnable task.  */
}

/*
 * i__schedule() is the main scheduler function.
 *
 * This function is heavily based on Linux 7.2 kernel scheduler.
 * The scheduler is primarily driven through the following execution paths:
 *
 * 1. Explicit blocking: waitqueue.
 *
 *    A task may voluntarily block by waiting on a waitqueue. It is then
 *    that the control is yielded back to the scheduler. The task remains
 *    ineligible for execution until it is explicitly awakened.
 *
 * 2. Wakeups won't cause an entry into the scheduler. A task will be added
 *    to the run-queue and will wait to be scheduled the next cycle.
 *
 *    For now, there's no preemption mechanism built for this scheduler.
 *    This means tasks are executed from beginning to end, that is if the
 *    task doesn't yield.
 */
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

		rq = context_switch(rq, prev, next);
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

void schedule_idle(void)
{
	do {
		__schedule(SM_IDLE);
	} while (need_resched());
}
