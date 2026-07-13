#define IDSCHED_IMPLEMENTATION
#include "idsched.h"
#include <stdio.h>

static void task_a(void *arg) { (void)arg; printf("running A\n"); }
static void task_b(void *arg) { (void)arg; printf("running B\n"); }
static void task_c(void *arg) { (void)arg; printf("running C\n"); }

int main(void)
{
	idsched_t       *sched;
	idsched_task_t  *a, *b, *c;
	idsched_task_t  *tasks[3];

	sched = idsched_create();

	a = idsched_task_create(sched, task_a, NULL);
	b = idsched_task_create(sched, task_b, NULL);
	c = idsched_task_create(sched, task_c, NULL);

	idsched_add_dep(b, a);

	tasks[0] = c;
	tasks[1] = a;
	tasks[2] = b;

	idsched_run_all(sched, tasks, 3);

	idsched_task_destroy_all(tasks, 3);
	idsched_destroy(sched);

	return 0;
}
