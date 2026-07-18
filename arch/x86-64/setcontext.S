#include <asm/unistd.h>
#include <x86-64/i_context_asm.h>
	.text
	.global isetcontext
	.type isetcontext, @function
isetcontext:
	/* Save argument since syscall will destroy it.  */
	pushq	%rdi

	/* Set the signal mask with
	   rt_sigprocmask (SIG_SETMASK, mask, NULL, _NSIG/8).  */
	leaq	IC_SIGMASK(%rdi), %rsi
	xorl	%edx, %edx
	movl	$SIG_SETMASK, %edi
	movl	$IC_NSIG8, %r10d
	movl	$__NR_rt_sigprocmask, %eax
	syscall
	/* Pop the pointer into RDX.  */
	popq	%rdx
	cmpq	$-4095, %rax		/* Check %rax for error.  */
	jae	isetcontext_error	/* Jump to error handler if error.  */

	/* Restore the floating-point context.  */
	fxrstor	IC_FPREGS(%rdx)

	/* Restore flags.  */
	pushq	IC_GREGS + (IMREG_RFLAGS * 8)(%rdx)
		popfq

	/* Load the new stack pointer and the preserved registers.  */
	movq	IC_GREGS + (IMREG_RSP * 8)(%rdx), %rsp
	movq	IC_GREGS + (IMREG_RBX * 8)(%rdx), %rbx
	movq	IC_GREGS + (IMREG_RBP * 8)(%rdx), %rbp
	movq	IC_GREGS + (IMREG_R12 * 8)(%rdx), %r12
	movq	IC_GREGS + (IMREG_R13 * 8)(%rdx), %r13
	movq	IC_GREGS + (IMREG_R14 * 8)(%rdx), %r14
	movq	IC_GREGS + (IMREG_R15 * 8)(%rdx), %r15

	/* The following ret should return to the address set with
	   getcontext. Therefore push the address on the stack.  */
	movq	IC_GREGS + (IMREG_RIP * 8)(%rdx), %rcx
	pushq	%rcx

	/* Setup registers used for passing args.  */
	movq	IC_GREGS + (IMREG_RDI * 8)(%rdx), %rdi
	movq	IC_GREGS + (IMREG_RSI * 8)(%rdx), %rsi
	movq	IC_GREGS + (IMREG_RCX * 8)(%rdx), %rcx
	movq	IC_GREGS + (IMREG_R8 * 8)(%rdx), %r8
	movq	IC_GREGS + (IMREG_R9 * 8)(%rdx), %r9

	/* Setup finally %rdx.  */
	movq	IC_GREGS + (IMREG_RDX * 8)(%rdx), %rdx

	/* Clear rax to indicate success.  */
	xorl	%eax, %eax
	ret

isetcontext_error:
	negq	%rax
	ret
