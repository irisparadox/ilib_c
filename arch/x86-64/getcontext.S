#include <asm/unistd.h>
#include <x86-64/i_context_asm.h>
	.text
	.global igetcontext
	.type igetcontext, @function
igetcontext:
	/* Save the preserved registers, the registers used for passing
	   args, and the return address.  */
	movq	%rbx, oiRBX(%rdi)
	movq	%rbp, oiRBP(%rdi)
	movq	%r12, oiR12(%rdi)
	movq	%r13, oiR13(%rdi)
	movq	%r14, oiR14(%rdi)
	movq	%r15, oiR15(%rdi)

	movq	%rdi, oiRDI(%rdi)
	movq	%rsi, oiRSI(%rdi)
	movq	%rdx, oiRDX(%rdi)
	movq	%rcx, oiRCX(%rdi)
	movq	%r8, oiR8(%rdi)
	movq	%r9, oiR9(%rdi)

	movq	(%rsp), %rcx
	movq	%rcx, oiRIP(%rdi)
	leaq	8(%rsp), %rcx		/* Exclude the return address.  */
	movq	%rcx, oiRSP(%rdi)

	/* Save flags.  */
	pushfq
	popq	%rcx
	movq	%rcx, oiRFLAGS(%rdi)

	/* Save the floating-point/SSE state.  */
	fxsave	oiFPREGS(%rdi)

	/* Save the current signal mask with
	   rt_sigprocmask (SIG_BLOCK, NULL, set, IC_NSIG8).  */
	leaq	oiSIGMASK(%rdi), %rdx
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
