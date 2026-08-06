#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <deftypei.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <linux/prctl.h>
#include <arch/stack.h>
#include <asm/switch_to.h>
#include <kinclude/kfun.h>
#include <kinclude/task.h>
#include <kinclude/sched.h>
#include <kinclude/kerrno.h>
#include <kinclude/syscall.h>

void free_task(struct itask *p)
{
	if (p->stack)
		munmap(p->stack, KSTACK_SIZE);

	if (p->tid)
		free_tid(p->tid);

	free(p);
}

struct itask *dup_task_struct(struct itask *p)
{
	struct itask *new;

	new = calloc(1, sizeof(struct itask));
	if (unlikely(new == NULL))
		return ERR_PTR(-ENOMEM);

	memcpy(new, p, sizeof(struct itask));

	new->stack = mmap(NULL, KSTACK_SIZE,
			  PROT_READ | PROT_WRITE,
			  MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK,
			  -1, 0);

	if (unlikely(new->stack == MAP_FAILED)) {
		free(new);
		return ERR_PTR(-ENOMEM);
	}

	return new;
}

struct itask *copy_task(struct ktid *ktid, struct clone_args *args)
{
	/* TODO Fix ordering hazards, ktid allocation and flags */
	struct itask *p;
	int retval;

	p = dup_task_struct(current);
	if (IS_ERR(p))
		return p;

	retval = copy_thread(p, args);
	if (retval)
		goto err;

	ktid = ktid_alloc();
	if (!ktid) {
		retval = -EAGAIN;
		goto err;
	}

	p->__state = TASK_NEW;
	p->on_rq   = TASK_ON_RQ_NONE;
	
	p->parent  = current;

	ilisti_init(&p->children);
	ilisti_init(&p->sibling);
	ilisti_init(&p->run_list);
	
	init_waitqueue_head(&p->wait_chldexit);
	init_waitqueue_entry(&p->wait_entry, p);

	pthread_mutex_init(&p->sleep_lock, NULL);
	pthread_cond_init(&p->sleep_signal, NULL);

	p->exec_start       = 0;
	p->sum_exec_runtime = 0;
	p->exit_state       = 0;
	p->exit_code        = 0;
	p->nvcsw            = 0;
	p->nivcsw           = 0;

	if (args->name)
		snprintf(p->comm, sizeof(p->comm), "%s", args->name);

	char vma_name[TASK_COMM_LEN + sizeof("/kstack")];
	snprintf(vma_name, sizeof(vma_name), "%s/kstack", p->comm);
	
	prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, p->stack, KSTACK_SIZE, vma_name);
	
	return p;

 err:
	free_task(p);
	return ERR_PTR(retval);
}

/*
 * This is the main fork routine.
 *
 * It creates a new task as a copy of the calling task,
 * makes it runnable, and returns its TID.
 */
tid_t kernel_clone(struct clone_args *args)
{
	struct ktid *ktid;
	struct itask *p;
	tid_t nr;

	p = copy_task(NULL, args);
	if (IS_ERR(p))
		return PTR_ERR(p);

	ktid = get_task_ktid(p);
	nr = ktid_nr(ktid);

	wake_up_new_task(p);

	return nr;
}

/*
 * Create a kernel thread.
 */
tid_t kernel_thread(int (*fn)(void *), void *arg, const char *name)
{
	struct clone_args args = {
		.name		= name,
		.fn		= fn,
		.fn_arg		= arg,
	};

	return kernel_clone(&args);
}

/*
 * Create a user mode thread.
 */
tid_t user_mode_thread(int (*fn)(void *), void *arg)
{
	struct clone_args args = {
		.fn		= fn,
		.fn_arg		= arg,
	};

	return kernel_clone(&args);
}

/**
 * sys_clone3 - create a new task
 * @uargs: pointer to the clone arguments
 * @size:  size of @uargs
 *
 * clone3() creates a new task by duplicating the calling task's
 * execution context and preparing a fresh kernel stack for it.
 * The syscall accepts a versioned argument structure whose size
 * is validated for forward compatibility.
 *
 * Return: On success, the TID of the newly created task.
 *         On error, a negative errno number.
 */
SYSCALL_DEFINE2(clone3, struct clone_args *, uargs, ilib_size_t, size)
{
	struct clone_args args;

	if (!uargs)
		return -EINVAL;

	if (size != sizeof(struct clone_args))
		return -EINVAL;

	args = *uargs;

	return kernel_clone(&args);
}
