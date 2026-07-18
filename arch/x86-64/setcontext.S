#include <x86-64/i_context_asm.h>

	.text

	.global isetcontext
	.type isetcontext, @function

isetcontext:
	# Restore FP/SSE state
	fxrstor IC_FPREGS(%rdi)

	# Save context pointer in r11
	movq %rdi, %r11

	# Save target RSP and RIP on current stack
	pushq IC_GREGS + (IMREG_RSP * 8)(%r11)
	pushq IC_GREGS + (IMREG_RIP * 8)(%r11)

	# Restore flags
	pushq IC_GREGS + (IMREG_RFLAGS * 8)(%r11)
	popfq

	# Restore registers that do not destroy our context pointer
	movq IC_GREGS + (IMREG_R8 * 8)(%r11), %r8
	movq IC_GREGS + (IMREG_R9 * 8)(%r11), %r9
	movq IC_GREGS + (IMREG_R10 * 8)(%r11), %r10
	movq IC_GREGS + (IMREG_R12 * 8)(%r11), %r12
	movq IC_GREGS + (IMREG_R13 * 8)(%r11), %r13
	movq IC_GREGS + (IMREG_R14 * 8)(%r11), %r14
	movq IC_GREGS + (IMREG_R15 * 8)(%r11), %r15

	movq IC_GREGS + (IMREG_RBX * 8)(%r11), %rbx
	movq IC_GREGS + (IMREG_RBP * 8)(%r11), %rbp

	movq IC_GREGS + (IMREG_RDX * 8)(%r11), %rdx
	movq IC_GREGS + (IMREG_RCX * 8)(%r11), %rcx
	movq IC_GREGS + (IMREG_RSI * 8)(%r11), %rsi

	# Restore RDI and RAX last because they are used by the ABI
	movq IC_GREGS + (IMREG_RDI * 8)(%r11), %rdi
	movq IC_GREGS + (IMREG_RAX * 8)(%r11), %rax

	# Recover saved RIP/RSP from the old stack
	popq %r11
	popq %rsp

	# Jump into restored context
	jmp *%r11
