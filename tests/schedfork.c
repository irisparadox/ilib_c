#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define IDSCHED_IMPLEMENTATION
#include <idsched.h>

static int forktask(void *arg)
{
	int x = 42;
	int *px = &x;

	printf("before fork: &x=%p px=%p *px=%d\n",
		(void *)&x, (void *)px, *px);

	idsched_task_t *child = idsched_task_fork();

	if (child == NULL) {
		printf("child: &x=%p px=%p *px=%d\n",
			(void *)&x, (void *)px, *px);
	} else {
		printf("parent: &x=%p px=%p *px=%d\n",
			(void *)&x, (void *)px, *px);
	}

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
