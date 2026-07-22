#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define IDSCHED_IMPLEMENTATION
#include <idsched.h>

static int printcounter(void *arg)
{
	(void)arg;

	for (int i = 0; i < 10; ++i) {
		printf("%d\n", i);
	}

	return 0;
}

static int forktask(void *arg)
{
	idsched_task_t *chld;
	for (int i = 0; i < 4; ++i) {
		chld = idsched_task_fork();
		if (chld == NULL) {
			printf("Child %d\n", i);
			return 0;
		} else {
			printf("Parent %d\n", i);
		}
	}

	while(idsched_task_wait(NULL) != -1);
	printf("Parent is done\n");
	return 0;
}

static int parent(void *arg)
{
	if (idsched_task_fork() == NULL) {
		for (int i = 0; i < 10; ++i)
			printf("%d\n", i);

		return 0;
	}

	printf("Parent exits\n");
	return 0;
}

static int parent2(void *arg)
{
	idsched_task_t *child;
	int status;

	child = idsched_task_fork();

	if (child == NULL)
		return 42;

	idsched_task_waittask(child, &status);

	printf("child exited: %d\n", status);

	return 0;
}

int main(void)
{
	idsched_t sched;
	idsched_task_t task;
	int status;

	assert(idsched_create(&sched, 2) == 0);
	assert(idsched_core_startup(&sched, 1) == 1);

	assert(idsched_task_create(&sched, &task, parent2, NULL) != IDSCHED_INVALID_TID);
	assert(idsched_task_submit(&sched, &task) == 0);

	assert(idsched_task_waittask(&task, &status) == 0);
	assert(status == 0);

	assert(idsched_task_destroy(&task) == 0);
	assert(idsched_core_shutdown(&sched, 1) == 1);
	assert(idsched_destroy(&sched) == 0);

	printf("All tests passed\n");
	return 0;
}
