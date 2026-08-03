#include <stddef.h>
#include <kinclude/sched.h>

/* char (*x)[offsetof(struct itask, thread_ctx)] = 1; */
/* char (*y)[offsetof(icontext_t, ic_stack.ss_sp)] = 1; */

/* char (*x)[offsetof(struct itask, thread)] = 1; */
/* char (*y)[offsetof(struct ithread_struct, sp)] = 1; */
