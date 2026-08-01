	.text
	.global ientry_SYSCALL_64
	.type ientry_SYSCALL_64, @function
ientry_SYSCALL_64:
	movq	%rsp, %fs:user_sp_scratch@tpoff
	movq	%fs:kernel_sp_scratch@tpoff, %rsp
	/* now on kernel stack, no register touched */

	pushq	%fs:user_sp_scratch@tpoff	/* sp */
	pushq	%r11				/* flags */
	pushq	%rcx				/* ip */
	pushq	%rax				/* orig_ax */
	pushq	%rdi				/* di */
	pushq	%rsi				/* si */
	pushq	%rdx				/* dx */
	pushq	%rcx				/* cx  — duplicates ip */
	pushq	%rax				/* ax  — duplicates orig_ax */
	pushq	%r8				/* r8 */
	pushq	%r9				/* r9 */
	pushq	%r10				/* r10 — real arg3 */
	pushq	%r11				/* r11 — duplicates flags */
	pushq	%rbx				/* bx */
	pushq	%rbp				/* bp */
	pushq	%r12				/* r12 */
	pushq	%r13				/* r13 */
	pushq	%r14				/* r14 */
	pushq	%r15				/* r15 */

	movq	%rsp, %rdi			/* &ipt_regs for the dispatcher */
	call	ido_syscall_64

iexit_SYSCALL_64: /* label is only there for debug purposes.  */
	/* We succeeded! Our job now is to pop
	   the regs back.  */
	popq	%r15
	popq	%r14
	popq	%r13
	popq	%r12
	popq	%rbp
	popq	%rbx

	/* General purpose regs.  */
	popq	%r11
	popq	%r10
	popq	%r9
	popq	%r8
	popq	%rax
	popq	%rcx
	popq	%rdx
	popq	%rsi
	popq	%rdi
	addq	$8, %rsp	/* discard orig_ax - not needed.  */
	popq	%rcx
	popq	%r11
	popq	%rsp

	/* sysretq */
	jmp	*%rcx
