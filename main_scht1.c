#define IDSCHED_IMPLEMENTATION
#include "idsched.h"

#include <assert.h>
#include <stdio.h>
#include <stdatomic.h>
#include <time.h>


#define NCORES 4
#define NTASKS 32


static atomic_int executed;
static atomic_int running;
static atomic_int max_running;


/* ---------------------------------------------------------- */

static void sleep_ms(long ms)
{
	struct timespec ts;

	ts.tv_sec  = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;

	nanosleep(&ts, NULL);
}


/* ---------------------------------------------------------- */

static int test_task(void *arg)
{
	int current;
	int max;

	(void)arg;

	printf("executing task\n");

	current = atomic_fetch_add(&running, 1) + 1;

	max = atomic_load(&max_running);

	while (current > max &&
	       !atomic_compare_exchange_weak(&max_running,
	                                     &max,
	                                     current));

	sleep_ms(10);

	atomic_fetch_add(&executed, 1);
	atomic_fetch_sub(&running, 1);

	printf("finishing task\n");

	return 0;
}


/* ---------------------------------------------------------- */
/* Single worker core                                         */
/* ---------------------------------------------------------- */

static void test_single_worker(void)
{
	idsched_t sched;
	idsched_task_t task;
	idsched_tid tid;
	int status;


	atomic_store(&executed, 0);
	atomic_store(&running, 0);
	atomic_store(&max_running, 0);


	assert(idsched_create(&sched, 2) == 0);

	assert(idsched_core_startup(&sched, IDSCHED_ALL_CORES) == 1);


	tid = idsched_task_create(&sched,
	                          &task,
	                          test_task,
	                          NULL);

	assert(tid != IDSCHED_INVALID_TID);


	assert(idsched_task_submit(&sched, &task) == 0);

	printf("submitted task\n");


	assert(idsched_task_wait(&task, &status) == 0);

	assert(IDSCHED_WIFEXITED(status));
	assert(IDSCHED_WEXITSTATUS(status) == 0);

	assert(task.flags & IDSCHED_TASK_DONE);

	assert(atomic_load(&executed) == 1);
	assert(atomic_load(&max_running) == 1);


	printf("[PASS] single worker execution\n");


	assert(idsched_core_shutdown(&sched, IDSCHED_ALL_CORES) == 1);

	assert(idsched_task_destroy(&task) == 0);

	assert(idsched_destroy(&sched) == 0);
}


/* ---------------------------------------------------------- */
/* Multiple worker cores                                      */
/* ---------------------------------------------------------- */

static void test_multiple_workers(void)
{
	idsched_t sched;
	idsched_task_t tasks[NTASKS];
	idsched_tid tid;
	int status;
	int i;


	atomic_store(&executed, 0);
	atomic_store(&running, 0);
	atomic_store(&max_running, 0);


	assert(idsched_create(&sched, NCORES) == 0);

	assert(idsched_core_startup(&sched, IDSCHED_ALL_CORES)
	       == NCORES - 1);


	for (i = 0; i < NTASKS; ++i) {
		tid = idsched_task_create(&sched,
		                          &tasks[i],
		                          test_task,
		                          NULL);

		assert(tid != IDSCHED_INVALID_TID);

		assert(idsched_task_submit(&sched,
		                           &tasks[i]) == 0);
	}


	for (i = 0; i < NTASKS; ++i) {
		assert(idsched_task_wait(&tasks[i], &status) == 0);

		assert(IDSCHED_WIFEXITED(status));
		assert(IDSCHED_WEXITSTATUS(status) == 0);

		assert(tasks[i].flags & IDSCHED_TASK_DONE);
	}


	assert(atomic_load(&executed) == NTASKS);

	/*
	 * At least two workers should overlap.
	 */
	assert(atomic_load(&max_running) > 1);


	printf("[PASS] multiple worker execution\n");


	assert(idsched_core_shutdown(&sched, IDSCHED_ALL_CORES)
	       == NCORES - 1);


	for (i = 0; i < NTASKS; ++i)
		assert(idsched_task_destroy(&tasks[i]) == 0);


	assert(idsched_destroy(&sched) == 0);
}


/* ---------------------------------------------------------- */

int main(void)
{
	test_single_worker();

	test_multiple_workers();

	printf("All Phase 1 tests passed.\n");

	return 0;
}
