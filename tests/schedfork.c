#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define IDSCHED_IMPLEMENTATION
#include <idsched.h>

static int forktask(void *arg)
{
	printf("This is parent\n");

	idsched_task_t *child = idsched_task_fork();

	if (child == NULL) {
		printf("This is child\n");
		return 0;
	}

	printf("This is parent 2\n");

	/* Give the scheduler a chance to run the child. */
	idsched_task_yield();

	idsched_task_wait(child, NULL);
	idsched_task_destroy(child);
	free(child);
	return 0;
}

int main(void)
{
	idsched_t sched;
	idsched_task_t task;
	int status;

	assert(idsched_create(&sched, 2) == 0);
	assert(idsched_core_startup(&sched, 1) == 1);

	assert(idsched_task_create(&sched, &task, forktask, NULL) != IDSCHED_INVALID_TID);
	assert(idsched_task_submit(&sched, &task) == 0);

	assert(idsched_task_wait(&task, &status) == 0);
	assert(status == 0);

	assert(idsched_task_destroy(&task) == 0);

	printf("All tests passed\n");
	return 0;
}
