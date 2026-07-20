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
	idsched_task_t *t = idsched_task_fork();

	if (t == NULL) {
		idsched_task_exec(printcounter, NULL);
		printf("error\n");
	} else {
		printf("parent now waits\n");
		while(idsched_task_wait(t, NULL) == -1);
	}

	printf("Parent is done\n");
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
