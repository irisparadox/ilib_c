#ifndef ICONTEXT_H_
#define ICONTEXT_H_

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif  /* _POSIX_C_SOURCE */

#include <signal.h>

#if defined(__x86_64__)
#include <x86-64/i_context.h>
#else
#error "Unsupported architecture."
#endif

#include <deftypei.h>

typedef struct icontext_t {
	struct icontext_t *ic_link;
	sigset_t           ic_sigmask;
	stack_t            ic_stack;
	imcontext_t        ic_mcontext;
} icontext_t;

/* Gets the user context and stores it in the variable pointed to by ICP */
extern int igetcontext(icontext_t *icp);

/* Sets the user context using the variable pointed to by ICP */
extern int isetcontext(icontext_t *icp);

/* Save current context in the variable pointed to by SICP and set
   context from the variable pointed to by DICP.  */
extern int iswapcontext(icontext_t *restrict sicp,
			const icontext_t *restrict dicp);

/* Initialize context ICP to begin execution at function FUNC with
   ARGC arguments when the context is activated for the first time
   through isetcontext() or iswapcontext().

   The arguments passed to FUNC are taken from the variadic argument
   list and must match the calling convention supported by the target
   architecture.  */
extern void imakecontext(icontext_t *icp, void (*func)(void),
			 int argc, ...);

#endif // ICONTEXT_H_
