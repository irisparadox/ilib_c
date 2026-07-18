#include <asm/unistd.h>
#include <x86-64/i_context_asm.h>
	.text
	.global igetcontext
	.type igetcontext, @function
igetcontext:
	/* Save the preserved registers, the registers used for passing
	   args, and the return address.  */
	movq	%rbx, IC_GREGS + (IMREG_RBX * 8)(%rdi)
	movq	%rbp, IC_GREGS + (IMREG_RBP * 8)(%rdi)
	movq	%r12, IC_GREGS + (IMREG_R12 * 8)(%rdi)
	movq	%r13, IC_GREGS + (IMREG_R13 * 8)(%rdi)
	movq	%r14, IC_GREGS + (IMREG_R14 * 8)(%rdi)
	movq	%r15, IC_GREGS + (IMREG_R15 * 8)(%rdi)

	movq	%rdi, IC_GREGS + (IMREG_RDI * 8)(%rdi)
	movq	%rsi, IC_GREGS + (IMREG_RSI * 8)(%rdi)
	movq	%rdx, IC_GREGS + (IMREG_RDX * 8)(%rdi)
	movq	%rcx, IC_GREGS + (IMREG_RCX * 8)(%rdi)
	movq	%r8, IC_GREGS + (IMREG_R8 * 8)(%rdi)
	movq	%r9, IC_GREGS + (IMREG_R9 * 8)(%rdi)

	movq	(%rsp), %rcx
	movq	%rcx, IC_GREGS + (IMREG_RIP * 8)(%rdi)
	leaq	8(%rsp), %rcx		/* Exclude the return address.  */
	movq	%rcx, IC_GREGS + (IMREG_RSP * 8)(%rdi)

	/* Save flags.  */
	pushfq
	popq	%rcx
	movq	%rcx, IC_GREGS + (IMREG_RFLAGS * 8)(%rdi)

	/* Save the floating-point/SSE state.  */
	fxsave	IC_FPREGS(%rdi)

	/* Save the current signal mask with
	   rt_sigprocmask (SIG_BLOCK, NULL, set, IC_NSIG8).  */
	leaq	IC_SIGMASK(%rdi), %rdx
	xorl	%esi, %esi
#if SIG_BLOCK == 0
	xorl	%edi, %edi
#else
	movl	$SIG_BLOCK, %edi
#endif
	movl	$IC_NSIG8, %r10d
	movl	$__NR_rt_sigprocmask, %eax
	syscall
	cmpq	$-4095, %rax		/* Check %rax for error.  */
	jae	igetcontext_error	/* Jump to error handler if error.  */

	/* All done, return 0 for success.  */
	xorl	%eax, %eax
	ret

igetcontext_error:
	negq	%rax
	ret
