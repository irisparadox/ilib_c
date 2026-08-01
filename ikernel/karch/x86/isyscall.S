	.text
	.global isyscall
	.type isyscall, @function
isyscall:
	movq	%rdi, %rax		# nr
	movq	%rsi, %rdi		# a1
	movq	%rdx, %rsi		# a2
	movq	%rcx, %r10		# a3
	movq	%r8, %r8		# a4 in place
	movq	%r9, %r9		# a5 in place
	movq	(%rsp), %rcx		# ret address
	pushfq
	popq	%r11			# flags
	jmp	ientry_SYSCALL_64
