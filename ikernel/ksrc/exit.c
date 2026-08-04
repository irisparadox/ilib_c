#include <kinclude/wait.h>
#include <kinclude/kerrno.h>
#include <kinclude/ktid.h>
#include <kinclude/sched.h>
#include <kinclude/exit.h>
#include <kinclude/syscall.h>

static bool is_effectively_child(struct itask *target)
{
	return target->parent == current;
}

static int eligible_child(struct iwait_opts *wo, struct itask *p)
{
	switch (wo->wo_type) {
	case TIDTYPE_TID:
		return p == wo->wo_ktid->thread;

	case TIDTYPE_MAX:
		return 1;
	}

	return 0;
}

static int child_wait_callback(iwait_queue_entry_t *wait, unsigned mode, int sync, void *key)
{
	return default_wake_function(wait, mode, sync, key);
}

static int wait_task_zombie(struct iwait_opts *wo, struct itask *p)
{
	int status;
	tid_t tid = task_tid_nr(p);

	if (!likely(wo->wo_flags & WEXITED))
		return 0;

	if (icmpxchg(&p->exit_state, EXIT_ZOMBIE, EXIT_DEAD) != EXIT_ZOMBIE)
		return 0;

	read_unlock(&tasklist_lock);
	/*
	 * We own this task now, nobody can reap it.
	 */
	status = p->exit_code;
	wo->wo_stat = status;

	write_lock(&tasklist_lock);
	ilisti_remove(&p->sibling);
	write_unlock(&tasklist_lock);

	kreaper_enqueue(kernel_reaper(), p);

	return tid;
}

static int wait_consider_task(struct iwait_opts *wo, struct itask *p)
{
	int exit_state = IREAD_ONCE(int, p->exit_state);
	int ret;

	if (unlikely(exit_state == EXIT_DEAD))
		return 0;

	ret = eligible_child(wo, p);
	if (!ret)
		return ret;

	if (exit_state == EXIT_ZOMBIE)
		return wait_task_zombie(wo, p);

	return 0;
}

static int do_wait_thread(struct iwait_opts *wo, struct itask *tsk)
{
	struct itask *p;

	list_for_each_entry(p, &tsk->children, sibling) {
		int ret = wait_consider_task(wo, p);

		if (ret)
			return ret;
	}

	return 0;
}

/*
 * This is an optimization for waiting on TIDTYPE_TID.
 * Removes the need to iterate through child list.
 */
static int do_wait_tid(struct iwait_opts *wo)
{
	struct itask *target;
	int retval;

	target = ktid_get_task(wo->wo_ktid);
	if (target && is_effectively_child(target)) {
		retval = wait_consider_task(wo, target);
		if (retval)
			return retval;
	}

	return 0;
}

long __do_wait(struct iwait_opts *wo)
{
	long retval;

	wo->notask_error = -ECHILD;
	if ((wo->wo_type < TIDTYPE_MAX) &&
	    (!wo->wo_ktid || !ktid_has_task(wo->wo_ktid)))
		goto notask;

	read_lock(&tasklist_lock);

	if (wo->wo_type == TIDTYPE_TID) {
		retval = do_wait_tid(wo);
		if (retval)
			return retval;
	} else {
		struct itask *tsk = current;

		retval = do_wait_thread(wo, tsk);
		if (retval)
			return retval;
	}
	read_unlock(&tasklist_lock);

 notask:
	retval = wo->notask_error;
	if (!retval && !(wo->wo_flags & WNOHANG))
		return -ERESTARTSYS;

	return retval;
}

static long do_wait(struct iwait_opts *wo)
{
	int retval;

	init_waitqueue_func_entry(&wo->child_wait, child_wait_callback);
	wo->child_wait.private = current;
	add_wait_queue(&current->wait_chldexit, &wo->child_wait);

	do {
		set_current_state(TASK_INTERRUPTIBLE);
		retval = __do_wait(wo);
		if (retval != -ERESTARTSYS)
			break;
		schedule();
	} while (1);

	__set_current_state(TASK_RUNNING);
	remove_wait_queue(&current->wait_chldexit, &wo->child_wait);
	return retval;
}

long kernel_wait3(tid_t utid, int *stat_addr, int options)
{
	struct iwait_opts wo;
	struct ktid *ktid = NULL;
	enum tid_type type;
	long ret;

	struct ktid_alloc *kal = ktid_get_allocator();
	if (utid == -1)
		type = TIDTYPE_MAX;
	else {
		type = TIDTYPE_TID;
		ktid = tid_get_ktid(kal, utid);
	}

	wo.wo_type  = type;
	wo.wo_ktid  = ktid;
	wo.wo_flags = options | WEXITED;
	wo.wo_stat  = 0;
	ret = do_wait(&wo);

	if (ret > 0 && stat_addr)
		*stat_addr = wo.wo_stat;

	return ret;
}

SYSCALL_DEFINE3(wait3, tid_t, utid, int *, stat_addr, int, options)
{
	long err = kernel_wait3(utid, stat_addr, options);

	return err;
}
