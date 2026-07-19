#include <deftypei.h>
#include <stdarg.h>
#include <icontext.h>

#include <x86-64/i_context.h>

/* This implementation can handle any ARGC value. Unlike POSIX's
   lazy decision to mark as a deprecated such wholesome library,
   we'll be making it portable by making this API able to handle
   64 bit arguments.
   imakecontext sets up a stack and the registers for the user
   context. The stack looks like this:
              +-----------------------+
              | next context          |
              +-----------------------+
              | parameter 7-n         |
              +-----------------------+
              | trampoline address    |
   %rsp ->    +-----------------------+

   The registers are set up like this:
     %rdi,%rsi,%rdx,%rcx,%r8,%r9: parameter 1 to 6
     %rbx    : address of next context
     %rsp    : stack pointer.
*/



void imakecontext(icontext_t *icp, void (*func)(void), int argc, ...)
{
	extern void __i___start_context(void) ILIB_HIDDEN;
	imreg_t *sp;
	ilib_size_t idx_ic_link;
	va_list ap;
	int i;

	/* Generate room in stack for parameter if needed and ic_link.  */
	sp = (imreg_t *) ((ilib_uintptr_t) icp->ic_stack.ss_sp
			  + icp->ic_stack.ss_size);
	sp -= (argc > 6 ? argc - 6 : 0) + 1;
	/* Align stack and make space for trampoline address.  */
	sp = (imreg_t *) ((((ilib_uintptr_t) sp) & -16L) - 8);


	idx_ic_link = (argc > 6 ? argc - 6 : 0) + 1;

	/* Setup context icp.  */
	/* Address to jump to.  */
	icp->ic_mcontext.gregs[IMREG_RIP] = (ilib_uintptr_t) func;
	/* Setup rbx.  */
	icp->ic_mcontext.gregs[IMREG_RBX] = (ilib_uintptr_t) &sp[idx_ic_link];
	icp->ic_mcontext.gregs[IMREG_RSP] = (ilib_uintptr_t) sp;

	/* Setup stack.  */
	sp[0] = (ilib_uintptr_t) &__i___start_context;
	sp[idx_ic_link] = (ilib_uintptr_t) icp->ic_link;

	va_start(ap, argc);
	/* Handle argv.

	   The System V AMD64 calling convention (SysV call convention)
	   is the contract that every function call on Linux/macOS/BSD
	   x86-64 follows. The standard specifies that the first 6
	   arguments are assigned respectively to: %rdi,%rsi,%rdx,%rcx,
	   %r8,%r9.

	   Anything beyond the 6th argument spills to the stack, pushed
	   right-to-left.  */
	for (i = 0; i < argc; ++i) {
		switch (i) {
		case 0:
			icp->ic_mcontext.gregs[IMREG_RDI] = va_arg(ap, imreg_t);
			break;
		case 1:
			icp->ic_mcontext.gregs[IMREG_RSI] = va_arg(ap, imreg_t);
			break;
		case 2:
			icp->ic_mcontext.gregs[IMREG_RDX] = va_arg(ap, imreg_t);
			break;
		case 3:
			icp->ic_mcontext.gregs[IMREG_RCX] = va_arg(ap, imreg_t);
			break;
		case 4:
			icp->ic_mcontext.gregs[IMREG_R8]  = va_arg(ap, imreg_t);
			break;
		case 5:
			icp->ic_mcontext.gregs[IMREG_R9]  = va_arg(ap, imreg_t);
			break;
		default:
			/* Put the value in stack */
			sp[i - 5] = va_arg(ap, imreg_t);
			break;
		}
	}
	va_end(ap);
}
