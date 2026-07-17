#define IDSCHED_PRED_MIN_HISTORY 4      /* shrink warmup so tests run fast */
#define IDSCHED_IMPLEMENTATION
#include "idsched.h"

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdatomic.h>
#include <time.h>

#define STALL_MS   80
#define D1_MS      5
#define D2_MS      150
#define SPACER_MS  2

#define VAR_FAST_MS 20
#define VAR_SLOW_MS 300

#define MAXSEQ 64

static atomic_int  exec_idx;
static idsched_tid exec_order[MAXSEQ];

static void sleep_ms(long ms)
{
	struct timespec ts;
	ts.tv_sec  = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;
	nanosleep(&ts, NULL);
}

static void record_exec(idsched_task_t *self)
{
	int idx = atomic_fetch_add(&exec_idx, 1);
	if (idx < MAXSEQ)
		exec_order[idx] = self->tid;
}

static int task_stall(void *arg)   { record_exec((idsched_task_t *)arg); sleep_ms(STALL_MS);  return 0; }
static int task_probeD1(void *arg) { record_exec((idsched_task_t *)arg); sleep_ms(D1_MS);      return 0; }
static int task_probeD2(void *arg) { record_exec((idsched_task_t *)arg); sleep_ms(D2_MS);      return 0; }
static int task_spacer(void *arg)  { record_exec((idsched_task_t *)arg); sleep_ms(SPACER_MS);  return 0; }
static int task_fresh(void *arg)   { record_exec((idsched_task_t *)arg); sleep_ms(D1_MS);      return 0; }

/* Used only by test_local_vs_global_prediction. Duration is passed
 * via arg so that a single function has BOTH fast and slow samples,
 * which is required to meaningfully distinguish local vs. global
 * prediction (the predictor is keyed per-function, not per-duration). */
static int task_variable(void *arg)
{
	sleep_ms((long)(intptr_t)arg);
	return 0;
}

/* Core routing is deterministic: target = tid % sch->online. With
 * online == 2, tid parity alone decides which core a task lands on.
 * Rather than hand-count parity across the whole test (fragile --
 * one missed/extra spacer desyncs everything downstream), track
 * the parity of the NEXT tid to be allocated and self-correct with
 * a throwaway spacer task whenever a desired parity doesn't match. */

static void ensure_next_tid_parity(idsched_t *sched, int *next_parity, int desired)
{
	idsched_task_t sp;
	idsched_tid    tid;
	int            status;

	if (*next_parity == desired)
		return;

	tid = idsched_task_create(sched, &sp, task_spacer, &sp);
	assert(tid != IDSCHED_INVALID_TID);
	assert(idsched_task_submit(sched, &sp) == 0);
	assert(idsched_task_wait(&sp, &status) == 0);
	idsched_task_destroy(&sp);

	*next_parity ^= 1;
}

static idsched_tid create_track(idsched_t *sched, idsched_task_t *t,
                                 int (*fn)(void *), void *arg, int *next_parity)
{
	idsched_tid tid = idsched_task_create(sched, t, fn, arg);
	assert(tid != IDSCHED_INVALID_TID);
	*next_parity ^= 1;
	return tid;
}

/* ------------------------------------------------------------
 * Verify FIFO replacement: with no predictor history, all
 * tasks tie on priority, so ordering must fall back to the
 * comparator's tid tie-break -- i.e. plain FIFO, matching Phase 1.
 * ------------------------------------------------------------ */
static void test_fifo_tiebreak_no_history(void)
{
	idsched_t      sched;
	idsched_task_t stall;
	idsched_task_t tasks[6];
	int            i, status;

	atomic_store(&exec_idx, 0);

	assert(idsched_create(&sched, 2) == 0);
	assert(idsched_core_startup(&sched, IDSCHED_ALL_CORES) == 1);

	assert(idsched_task_create(&sched, &stall, task_stall, &stall) != IDSCHED_INVALID_TID);
	assert(idsched_task_submit(&sched, &stall) == 0);

	for (i = 0; i < 6; ++i) {
		assert(idsched_task_create(&sched, &tasks[i], task_probeD1, &tasks[i]) != IDSCHED_INVALID_TID);
		assert(idsched_task_submit(&sched, &tasks[i]) == 0);
	}

	assert(idsched_task_wait(&stall, &status) == 0);
	for (i = 0; i < 6; ++i)
		assert(idsched_task_wait(&tasks[i], &status) == 0);

	for (i = 2; i <= 6; ++i)
		assert(exec_order[i - 1] < exec_order[i]);

	printf("[PASS] FIFO tie-break with no predictor history\n");

	assert(idsched_core_shutdown(&sched, IDSCHED_ALL_CORES) == 1);
	idsched_task_destroy(&stall);
	for (i = 0; i < 6; ++i) idsched_task_destroy(&tasks[i]);
	assert(idsched_destroy(&sched) == 0);
}

/* ------------------------------------------------------------
 * Verify priority queue ordering + priority immutability: once
 * the predictor has learned two clearly different runtimes,
 * queued instances must come out grouped by priority (never
 * interleaved), tie-broken by tid within a group, and a task's
 * stored priority must never change between submit and completion.
 * ------------------------------------------------------------ */
static void test_priority_ordering_with_history(void)
{
	idsched_t      sched;
	idsched_task_t warm[2 * IDSCHED_PRED_MIN_HISTORY];
	idsched_task_t stall;
	idsched_task_t d1[4], d2[4];
	int            i, status, d1_first;
	ilib_uint64_t  d1_prio, d2_prio;
	idsched_tid    slot1;

	atomic_store(&exec_idx, 0);

	assert(idsched_create(&sched, 2) == 0);
	assert(idsched_core_startup(&sched, IDSCHED_ALL_CORES) == 1);

	for (i = 0; i < IDSCHED_PRED_MIN_HISTORY; ++i) {
		assert(idsched_task_create(&sched, &warm[i], task_probeD1, &warm[i]) != IDSCHED_INVALID_TID);
		assert(idsched_task_submit(&sched, &warm[i]) == 0);
		assert(idsched_task_wait(&warm[i], &status) == 0);
	}
	for (i = 0; i < IDSCHED_PRED_MIN_HISTORY; ++i) {
		int j = IDSCHED_PRED_MIN_HISTORY + i;
		assert(idsched_task_create(&sched, &warm[j], task_probeD2, &warm[j]) != IDSCHED_INVALID_TID);
		assert(idsched_task_submit(&sched, &warm[j]) == 0);
		assert(idsched_task_wait(&warm[j], &status) == 0);
	}

	atomic_store(&exec_idx, 0);   /* discard warmup trace */

	assert(idsched_task_create(&sched, &stall, task_stall, &stall) != IDSCHED_INVALID_TID);
	assert(idsched_task_submit(&sched, &stall) == 0);

	for (i = 0; i < 4; ++i) {
		assert(idsched_task_create(&sched, &d1[i], task_probeD1, &d1[i]) != IDSCHED_INVALID_TID);
		assert(idsched_task_submit(&sched, &d1[i]) == 0);
		assert(idsched_task_create(&sched, &d2[i], task_probeD2, &d2[i]) != IDSCHED_INVALID_TID);
		assert(idsched_task_submit(&sched, &d2[i]) == 0);
	}

	d1_prio = d1[0].prio;
	d2_prio = d2[0].prio;
	assert(d1_prio != d2_prio);

	assert(idsched_task_wait(&stall, &status) == 0);
	for (i = 0; i < 4; ++i) {
		assert(idsched_task_wait(&d1[i], &status) == 0);
		assert(idsched_task_wait(&d2[i], &status) == 0);
	}

	/* priority immutability: unchanged after execution */
	assert(d1[0].prio == d1_prio);
	assert(d2[0].prio == d2_prio);

	/* Phase 1's plain-FIFO test already proved pqueue_top returns
	 * the minimum by this comparator's ordering (smaller tid runs
	 * first when tied), so smaller prio (shorter predicted D1)
	 * must run first here too. */
	slot1 = exec_order[1];
	d1_first = 0;
	for (i = 0; i < 4; ++i)
		if (slot1 == d1[i].tid) d1_first = 1;
	assert(d1_first == 1);

	for (i = 0; i < 4; ++i) assert(exec_order[1 + i] == d1[i].tid);
	for (i = 0; i < 4; ++i) assert(exec_order[5 + i] == d2[i].tid);

	printf("[PASS] priority ordering (short-predicted first) + immutability\n");

	assert(idsched_core_shutdown(&sched, IDSCHED_ALL_CORES) == 1);
	for (i = 0; i < 2 * IDSCHED_PRED_MIN_HISTORY; ++i) idsched_task_destroy(&warm[i]);
	idsched_task_destroy(&stall);
	for (i = 0; i < 4; ++i) { idsched_task_destroy(&d1[i]); idsched_task_destroy(&d2[i]); }
	assert(idsched_destroy(&sched) == 0);
}

/* ------------------------------------------------------------
 * Verify predictor updates occur only on completion: queue
 * several never-before-run instances of the same function
 * together (core stalled, none has completed) and confirm
 * none of them influenced each other's prediction while pending.
 * ------------------------------------------------------------ */
static void test_predictor_updates_only_on_completion(void)
{
	idsched_t      sched;
	idsched_task_t stall;
	idsched_task_t batch[5];
	idsched_task_t probe;
	int            i, status;

	assert(idsched_create(&sched, 2) == 0);
	assert(idsched_core_startup(&sched, IDSCHED_ALL_CORES) == 1);

	assert(idsched_task_create(&sched, &stall, task_stall, &stall) != IDSCHED_INVALID_TID);
	assert(idsched_task_submit(&sched, &stall) == 0);

	for (i = 0; i < 5; ++i) {
		assert(idsched_task_create(&sched, &batch[i], task_fresh, &batch[i]) != IDSCHED_INVALID_TID);
		assert(idsched_task_submit(&sched, &batch[i]) == 0);
		assert(batch[i].prio == IDSCHED_DEFAULT_RUNTIME);
	}

	assert(idsched_task_wait(&stall, &status) == 0);
	for (i = 0; i < 5; ++i)
		assert(idsched_task_wait(&batch[i], &status) == 0);

	assert(idsched_task_create(&sched, &probe, task_fresh, &probe) != IDSCHED_INVALID_TID);
	assert(idsched_task_submit(&sched, &probe) == 0);
	assert(probe.prio != IDSCHED_DEFAULT_RUNTIME);
	assert(idsched_task_wait(&probe, &status) == 0);

	printf("[PASS] predictor only updates after task completion\n");

	assert(idsched_core_shutdown(&sched, IDSCHED_ALL_CORES) == 1);
	idsched_task_destroy(&stall);
	for (i = 0; i < 5; ++i) idsched_task_destroy(&batch[i]);
	idsched_task_destroy(&probe);
	assert(idsched_destroy(&sched) == 0);
}

/* ------------------------------------------------------------
 * Verify EMA convergence: repeated identical-duration samples
 * should pull the predictor's estimate close to the true value.
 * ------------------------------------------------------------ */
static void test_ema_convergence(void)
{
	idsched_t      sched;
	idsched_task_t warm[IDSCHED_PRED_MIN_HISTORY];
	idsched_task_t probe;
	int            i, status;
	ilib_uint64_t  true_ns = (ilib_uint64_t)D1_MS * 1000000ULL;

	assert(idsched_create(&sched, 2) == 0);
	assert(idsched_core_startup(&sched, IDSCHED_ALL_CORES) == 1);

	for (i = 0; i < IDSCHED_PRED_MIN_HISTORY; ++i) {
		assert(idsched_task_create(&sched, &warm[i], task_probeD1, &warm[i]) != IDSCHED_INVALID_TID);
		assert(idsched_task_submit(&sched, &warm[i]) == 0);
		assert(idsched_task_wait(&warm[i], &status) == 0);
	}

	assert(idsched_task_create(&sched, &probe, task_probeD1, &probe) != IDSCHED_INVALID_TID);
	assert(idsched_task_submit(&sched, &probe) == 0);

	/* loose band: real sleeps jitter, this is a timing test */
	assert(probe.prio != IDSCHED_DEFAULT_RUNTIME);
	assert(probe.prio > true_ns / 3 && probe.prio < true_ns * 3);

	assert(idsched_task_wait(&probe, &status) == 0);

	printf("[PASS] EMA converges toward the true measured runtime\n");

	assert(idsched_core_shutdown(&sched, IDSCHED_ALL_CORES) == 1);
	for (i = 0; i < IDSCHED_PRED_MIN_HISTORY; ++i) idsched_task_destroy(&warm[i]);
	idsched_task_destroy(&probe);
	assert(idsched_destroy(&sched) == 0);
}

/* ------------------------------------------------------------
 * Verify local/global predictor policy.
 *
 * NOTE: the predictor is keyed per-function (see ihstmap_ptr_hash),
 * so the global predictor never blends across different functions --
 * gpred[task_probeD1] and gpred[task_probeD2] are entirely separate
 * entries. To meaningfully distinguish "local preferred" from
 * "global fallback" we need ONE function with TWO different observed
 * durations, split across two cores: task_variable, driven by arg.
 * ------------------------------------------------------------ */
static void test_local_vs_global_prediction(void)
{
	idsched_t      sched;
	idsched_task_t fast_warm[IDSCHED_PRED_MIN_HISTORY];
	idsched_task_t slow_probe_early;
	idsched_task_t slow_warm[IDSCHED_PRED_MIN_HISTORY - 1];
	idsched_task_t slow_probe_full;
	idsched_task_t fast_probe_check;
	int            i, status;
	idsched_tid    tid;
	int            next_parity = 0;   /* tid 0 is even -> first parity is 0 */

	assert(idsched_create(&sched, 3) == 0);
	assert(idsched_core_startup(&sched, IDSCHED_ALL_CORES) == 2);

	for (i = 0; i < IDSCHED_PRED_MIN_HISTORY; ++i) {
		ensure_next_tid_parity(&sched, &next_parity, 0);
		tid = create_track(&sched, &fast_warm[i], task_variable,
		                    (void *)(intptr_t)VAR_FAST_MS, &next_parity);
		assert(tid % 2 == 0);
		assert(idsched_task_submit(&sched, &fast_warm[i]) == 0);
		assert(idsched_task_wait(&fast_warm[i], &status) == 0);
	}

	ensure_next_tid_parity(&sched, &next_parity, 1);
	tid = create_track(&sched, &slow_probe_early, task_variable,
	                    (void *)(intptr_t)VAR_SLOW_MS, &next_parity);
	assert(tid % 2 == 1);
	assert(idsched_task_submit(&sched, &slow_probe_early) == 0);
	assert(slow_probe_early.prio != IDSCHED_DEFAULT_RUNTIME);
	assert(slow_probe_early.prio < (ilib_uint64_t)VAR_FAST_MS * 1000000ULL * 5);
	assert(idsched_task_wait(&slow_probe_early, &status) == 0);

	for (i = 0; i < IDSCHED_PRED_MIN_HISTORY - 1; ++i) {
		ensure_next_tid_parity(&sched, &next_parity, 1);
		tid = create_track(&sched, &slow_warm[i], task_variable,
		                    (void *)(intptr_t)VAR_SLOW_MS, &next_parity);
		assert(tid % 2 == 1);
		assert(idsched_task_submit(&sched, &slow_warm[i]) == 0);
		assert(idsched_task_wait(&slow_warm[i], &status) == 0);
	}

	ensure_next_tid_parity(&sched, &next_parity, 1);
	tid = create_track(&sched, &slow_probe_full, task_variable,
	                    (void *)(intptr_t)VAR_SLOW_MS, &next_parity);
	assert(tid % 2 == 1);
	assert(idsched_task_submit(&sched, &slow_probe_full) == 0);
	assert(slow_probe_full.prio > (ilib_uint64_t)VAR_SLOW_MS * 1000000ULL * 6 / 10);
	assert(idsched_task_wait(&slow_probe_full, &status) == 0);

	ensure_next_tid_parity(&sched, &next_parity, 0);
	tid = create_track(&sched, &fast_probe_check, task_variable,
	                    (void *)(intptr_t)VAR_FAST_MS, &next_parity);
	assert(tid % 2 == 0);
	assert(idsched_task_submit(&sched, &fast_probe_check) == 0);
	assert(fast_probe_check.prio < (ilib_uint64_t)VAR_FAST_MS * 1000000ULL * 5);
	assert(idsched_task_wait(&fast_probe_check, &status) == 0);

	printf("[PASS] local predictor preferred when sufficient; global fallback otherwise\n");

	assert(idsched_core_shutdown(&sched, IDSCHED_ALL_CORES) == 2);
	for (i = 0; i < IDSCHED_PRED_MIN_HISTORY; ++i) idsched_task_destroy(&fast_warm[i]);
	for (i = 0; i < IDSCHED_PRED_MIN_HISTORY - 1; ++i) idsched_task_destroy(&slow_warm[i]);
	idsched_task_destroy(&slow_probe_early);
	idsched_task_destroy(&slow_probe_full);
	idsched_task_destroy(&fast_probe_check);
	assert(idsched_destroy(&sched) == 0);
}

int main(void)
{
	test_fifo_tiebreak_no_history();
	test_priority_ordering_with_history();
	test_predictor_updates_only_on_completion();
	test_ema_convergence();
	test_local_vs_global_prediction();
	printf("All Phase 2 tests passed.\n");
	return 0;
}
