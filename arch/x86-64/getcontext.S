#include <x86-64/i_context_asm.h>

	.text

	.global igetcontext
	.type igetcontext, @function

igetcontext:
	# save general purpose registers
	movq %r8,  IC_GREGS + (IMREG_R8 * 8)(%rdi)
	movq %r9,  IC_GREGS + (IMREG_R9 * 8)(%rdi)
	movq %r10, IC_GREGS + (IMREG_R10 * 8)(%rdi)
	movq %r11, IC_GREGS + (IMREG_R11 * 8)(%rdi)
	movq %r12, IC_GREGS + (IMREG_R12 * 8)(%rdi)
	movq %r13, IC_GREGS + (IMREG_R13 * 8)(%rdi)
	movq %r14, IC_GREGS + (IMREG_R14 * 8)(%rdi)
	movq %r15, IC_GREGS + (IMREG_R15 * 8)(%rdi)

	movq %rdi, IC_GREGS + (IMREG_RDI * 8)(%rdi)
	movq %rsi, IC_GREGS + (IMREG_RSI * 8)(%rdi)
	movq %rbp, IC_GREGS + (IMREG_RBP * 8)(%rdi)
	movq %rbx, IC_GREGS + (IMREG_RBX * 8)(%rdi)
	movq %rdx, IC_GREGS + (IMREG_RDX * 8)(%rdi)
	movq %rax, IC_GREGS + (IMREG_RAX * 8)(%rdi)
	movq %rcx, IC_GREGS + (IMREG_RCX * 8)(%rdi)

	# save return address
	movq (%rsp), %rax
	movq %rax, IC_GREGS + (IMREG_RIP * 8)(%rdi)

	# save caller stack pointer
	leaq 8(%rsp), %rax
	movq %rax, IC_GREGS + (IMREG_RSP * 8)(%rdi)

	# save flags
	pushfq
	popq %rax
	movq %rax, IC_GREGS + (IMREG_RFLAGS * 8)(%rdi)

	# save x87/MMX/SSE state
	fxsave IC_FPREGS(%rdi)

	xorl %eax, %eax
	ret
