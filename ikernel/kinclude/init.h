#ifndef INIT_H_
#define INIT_H_

#define INIT_TASK_COMM "swapper"

struct itask; /* thank you C for the forward declarations.  */

extern void kernel_smp_start(int (*entry)(void *), void *arg);

extern void cpu_idle_prepare(void);
extern void cpu_init_prepare(void);

#endif // INIT_H_
