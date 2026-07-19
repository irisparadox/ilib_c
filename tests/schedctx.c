#include <stdio.h>
#include <assert.h>

#define IDSCHED_IMPLEMENTATION
#include <idsched.h>

static int second(void *arg)
{
	printf("second\n");
	return 42;
}

static int first(void *arg)
{
	printf("first\n");

	idsched_task_exec(second, NULL);

	printf("unreachable\n");

	return 0;
}

static int counterA(void *arg)
{
	for (int i = 0; i < 15; ++i) {
		printf("A %d\n", i);
		idsched_task_yield();
	}

	return 0;
}

static int counterB(void *arg)
{
	for (int i = 0; i < 15; ++i) {
		printf("B %d\n", i);
		idsched_task_yield();
	}

	return 0;
}

static int hello(void *arg)
{
	printf("Hello from task: arg=%p\n", arg);
	return 42;
}

int main(void)
{
	idsched_t sched;
	idsched_task_t task1, task2, task3, task4;
	int status1, status2, status3, status4;

	assert(idsched_create(&sched, 2) == 0);
	assert(idsched_core_startup(&sched, 1) == 1);

	assert(idsched_task_create(&sched, &task1, hello, &task1) != IDSCHED_INVALID_TID);
	assert(idsched_task_create(&sched, &task2, counterA, &task2) != IDSCHED_INVALID_TID);
	assert(idsched_task_create(&sched, &task3, counterB, &task3) != IDSCHED_INVALID_TID);
	assert(idsched_task_create(&sched, &task4, first, &task4) != IDSCHED_INVALID_TID);

	assert(idsched_task_submit(&sched, &task1) == 0);
	assert(idsched_task_submit(&sched, &task2) == 0);
	assert(idsched_task_submit(&sched, &task3) == 0);
	assert(idsched_task_submit(&sched, &task4) == 0);

	assert(idsched_task_wait(&task1, &status1) == 0);
	assert(idsched_task_wait(&task2, &status2) == 0);
	assert(idsched_task_wait(&task3, &status3) == 0);
	assert(idsched_task_wait(&task4, &status4) == 0);
	assert(status1 == 42);
	assert(status2 == 0);
	assert(status3 == 0);
	assert(status4 == 42);

	printf("Tasks finished successfully.\n");
	return 0;
}
