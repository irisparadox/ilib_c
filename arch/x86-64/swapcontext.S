#include <asm/unistd.h>
#include <x86-64/i_context_asm.h>
	.text
	.global iswapcontext
	.type iswapcontext, @function
iswapcontext:
	/* Save the preserved registers, the registers used for passing args,
	   and the return address.  */
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
	leaq	8(%rsp), %rcx				/* Exclude the return address.  */
	movq	%rcx, oiRSP(%rdi)

	pushfq
	popq	%rax
	movq	%rax, oiRFLAGS(%rdi)
	/* x87/MMX/SSE state.  */
	fxsave oiFPREGS(%rdi)

	/* The syscall destroys some registers, save them.  */
	movq	%rsi, %r12
	movq	%rdi, %r9

	/* Save the current signal mask and install the new one with
	   rt_sigprocmask (SIG_BLOCK, newset, oldset,_NSIG/8).  */
	leaq	oiSIGMASK(%rdi), %rdx
	leaq	oiSIGMASK(%rsi), %rsi
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

SYSCALL_SWPCTX_ERRL:
	negq	%rax
	ret
