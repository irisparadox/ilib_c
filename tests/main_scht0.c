#define IDSCHED_IMPLEMENTATION
#include "idsched.h"

#include <assert.h>
#include <stdio.h>

#define NCORES 4

static void test_fixed_ncores(idsched_t *sched)
{
	assert(sched->ncores == NCORES);
	printf("[PASS] fixed number of cores\n");
}

static void test_one_queue_per_core(idsched_t *sched)
{
	ilib_size_t i;

	for (i = 0; i < sched->ncores; ++i)
		assert(sched->cores[i].rq.cmp == i_task_cmp);

	printf("[PASS] one queue per core\n");
}

static void test_one_pthread_per_core(idsched_t *sched)
{
	ilib_size_t i, j;
	int         started;

	started = idsched_core_startup(sched, IDSCHED_ALL_CORES);
	assert(started == (int)(sched->ncores - 1));

	/* every worker core is ONLINE and has a distinct thread id */
	for (i = 1; i < sched->ncores; ++i) {
		assert(sched->cores[i].flags & IDSCHED_CORE_ONLINE);

		for (j = i + 1; j < sched->ncores; ++j)
			assert(!pthread_equal(sched->cores[i].thread, sched->cores[j].thread));
	}

	printf("[PASS] one pthread per worker core\n");
}

int main(void)
{
	idsched_t sched;

	assert(idsched_create(&sched, NCORES) == 0);

	test_fixed_ncores(&sched);
	test_one_queue_per_core(&sched);
	test_one_pthread_per_core(&sched);

	assert(idsched_core_shutdown(&sched, IDSCHED_ALL_CORES) == (int)(sched.ncores - 1));
	assert(idsched_destroy(&sched) == 0);

	printf("All Phase 0 tests passed.\n");
	return 0;
}
