#ifndef KCONF_H_
#define KCONF_H_

/*
 * Number of virtual cpus to run
 * in the kernel.
 *
 * Each virtual cpu is a posix-thread
 * instance.
 *
 * Default: 16 virtual cpus.
 */
#ifndef NR_VCPU
#define NR_VCPU 16
#endif  /* NR_VCPU */

/*
 * Enable debug configurations.
 * By default this is turned OFF.
 *
 * Set to 1 if you want to customize
 * debug or enable tracing.
 *
 * Set to 0 for regultar use.
 */
#ifndef KENABLE_DEBUG_OPTS
#define KENABLE_DEBUG_OPTS 0
#endif  /* KENABLE_DEBUG_OPTS */

/*
 * Okay, so because stacks are allocated
 * using mmap, these sizes are based in
 * pages.
 *
 * For the kernel stack 16 KiB is good
 * enough, probably more than enough.
 *
 * For the user stack, 16 pages, which
 * is 64 KiB, probably more than we'll
 * ever need.
 */
#ifndef KSTACK_SIZE
#define KSTACK_SIZE (4 * 4096)
#endif  /* KSTACK_SIZE */

#ifndef USTACK_SIZE
#define USTACK_SIZE (16 * 4096)
#endif  /* USTACK_SIZE */

/*
 * This enables stats in the kernel.
 * Specifically, it enables the run-queue
 * stats:
 */
#ifndef KCONF_SCHED_STATS
#define KCONF_SCHED_STATS 0
#endif  /* KCONF_SCHED_STATS */


/*
 * Virtual CPU statistics.
 *
 * The feature is disabled by default,
 * but when enabled it tracks the virtual
 * runtime of the cpu, the number of tasks
 * that ran in this cpu, number of migrations,
 * balance stats, etc.
 */
#ifndef KCONF_VCPU_STATS
#define KCONF_VCPU_STATS 1
#endif  /* KCONF_VCPU_STATS */


#ifndef NICE_WIDTH
#define NICE_WIDTH 40
#endif  /* NICE_WIDTH */

#ifndef MAX_RT_PRIO
#define MAX_RT_PRIO 100
#endif  /* MAX_RT_PRIO */

#endif // KCONF_H_
