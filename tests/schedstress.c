#define IDSCHED_IMPLEMENTATION
#include "idsched.h"

#include <stdio.h>
#include <assert.h>

/* ---- Leaf task: just yields a few times then exits with its index ---- */
static int leafwork(void *arg)
{
	int idx = (int)(ilib_intptr_t)arg;

	for (int i = 0; i < 5; ++i) {
		printf("leaf[%d]: tick %d\n", idx, i);
		idsched_task_yield();
	}

	return idx;
}

/* ---- Nested fork: forks once more, then execs into leafwork ---- */
static int nestedforktask(void *arg)
{
	int base = (int)(ilib_intptr_t)arg;
	idsched_task_t *chld;

	chld = idsched_task_fork();
	if (chld == NULL) {
		/* grandchild */
		idsched_task_exec(leafwork, (void *)(ilib_intptr_t)(base + 100));
		/* unreachable */
	} else if (chld == IDSCHED_INVALID_CHILD) {
		printf("nestedforktask[%d]: fork failed\n", base);
		return -1;
	}

	/* parent-of-grandchild: yield a couple times before waiting */
	idsched_task_yield();
	idsched_task_yield();

	int status = -1;
	assert(idsched_task_waittask(chld, &status) == 0);
	printf("nestedforktask[%d]: grandchild exited status=%d\n", base, status);

	return base;
}

/* ---- Root task: forks N children, some plain leaves, one nested ---- */
#define NCHILDREN 4

static int roottask(void *arg)
{
	(void)arg;
	idsched_task_t *kids[NCHILDREN];
	int i;

	for (i = 0; i < NCHILDREN; ++i) {
		idsched_task_t *chld = idsched_task_fork();

		if (chld == NULL) {
			/* child */
			if (i == 0)
				idsched_task_exec(nestedforktask, (void *)(ilib_intptr_t)i);
			else
				idsched_task_exec(leafwork, (void *)(ilib_intptr_t)i);
			/* unreachable */
		} else if (chld == IDSCHED_INVALID_CHILD) {
			printf("roottask: fork %d failed\n", i);
			kids[i] = NULL;
		} else {
			kids[i] = chld;
			printf("roottask: spawned child %d\n", i);
		}

		idsched_task_yield();
	}

	/* Targeted wait4 on each child specifically, in reverse order,
	 * to make sure wait4 (not just wait-any) is exercised. */
	for (i = NCHILDREN - 1; i >= 0; --i) {
		int status = -1;

		if (kids[i] == NULL)
			continue;

		assert(idsched_task_waittask(kids[i], &status) == 0);
		printf("roottask: child %d exited status=%d\n", i, status);
	}

	/* Nothing left to reap: should immediately fail with ECHILD. */
	assert(idsched_task_wait(NULL) == -1);

	printf("roottask: all children reaped\n");
	return 0;
}

int main(void)
{
	idsched_t sched;
	idsched_task_t task;
	int status;

	assert(idsched_create(&sched, 2) == 0);
	assert(idsched_core_startup(&sched, IDSCHED_ALL_CORES) == 1);

	assert(idsched_task_create(&sched, &task, roottask, NULL) != IDSCHED_INVALID_TID);
	assert(idsched_task_submit(&sched, &task) == 0);

	assert(idsched_task_waittask(&task, &status) == 0);
	assert(status == 0);

	assert(idsched_task_destroy(&task) == 0);
	assert(idsched_core_shutdown(&sched, IDSCHED_ALL_CORES) == 1);
	assert(idsched_destroy(&sched) == 0);

	printf("All tests passed\n");
	return 0;
}
