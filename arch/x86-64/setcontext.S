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
	leaq	oiSIGMASK(%rdi), %rsi
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
	fxrstor	oiFPREGS(%rdx)

	/* Restore flags.  */
	pushq	oiRFLAGS(%rdx)
		popfq

	/* Load the new stack pointer and the preserved registers.  */
	movq	oiRSP(%rdx), %rsp
	movq	oiRBX(%rdx), %rbx
	movq	oiRBP(%rdx), %rbp
	movq	oiR12(%rdx), %r12
	movq	oiR13(%rdx), %r13
	movq	oiR14(%rdx), %r14
	movq	oiR15(%rdx), %r15

	/* The following ret should return to the address set with
	   getcontext. Therefore push the address on the stack.  */
	movq	oiRIP(%rdx), %rcx
	pushq	%rcx

	/* Setup registers used for passing args.  */
	movq	oiRDI(%rdx), %rdi
	movq	oiRSI(%rdx), %rsi
	movq	oiRCX(%rdx), %rcx
	movq	oiR8(%rdx), %r8
	movq	oiR9(%rdx), %r9

	/* Setup finally %rdx.  */
	movq	oiRDX(%rdx), %rdx

	/* Clear rax to indicate success.  */
	xorl	%eax, %eax
	ret

isetcontext_error:
	negq	%rax
	ret
