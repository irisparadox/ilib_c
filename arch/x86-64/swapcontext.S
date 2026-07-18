#include <asm/unistd.h>
#include <x86-64/i_context_asm.h>
	.text
	.global iswapcontext
	.type iswapcontext, @function
iswapcontext:
	/* Save the preserved registers, the registers used for passing args,
	   and the return address.  */
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
	leaq	8(%rsp), %rcx				/* Exclude the return address.  */
	movq	%rcx, IC_GREGS + (IMREG_RSP * 8)(%rdi)

	pushfq
	popq	%rax
	movq	%rax, IC_GREGS + (IMREG_RFLAGS * 8)(%rdi)
	/* x87/MMX/SSE state.  */
	fxsave IC_FPREGS(%rdi)

	/* The syscall destroys some registers, save them.  */
	movq	%rsi, %r12
	movq	%rdi, %r9

	/* Save the current signal mask and install the new one with
	   rt_sigprocmask (SIG_BLOCK, newset, oldset,_NSIG/8).  */
	leaq	IC_SIGMASK(%rdi), %rdx
	leaq	IC_SIGMASK(%rsi), %rsi
	movl	$SIG_SETMASK, %edi
	movl	$IC_NSIG8,%r10d
	movl	$__NR_rt_sigprocmask, %eax
	syscall
	cmpq	$-4095, %rax		/* Check %rax for error.  */
	jae	SYSCALL_SWPCTX_ERRL	/* Jump to error handler if error.  */

	/* Restore destroyed register into RDX. Leaving RDI and RSI available
	   for use later can avoid shuffling values.  */
	movq	%r12, %rdx

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

SYSCALL_SWPCTX_ERRL:
	negq	%rax
	ret
