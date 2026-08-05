/*
 * x86-64 system call entry stub.
 *
 * This is the only entry point for all user-to-kernel transitions.
 * The stub switches execution from the caller's user stack to the
 * current task's kernel stack, constructs a complete pt_regs frame,
 * and transfers control to the generic syscall dispatcher.
 *
 * Unlike a real kernel, no privilege-level transition occurs. Instead,
 * the user/kernel boundary is emulated by maintaining separate stacks
 * for each task and explicitly switching between them.
 *
 * Calling convention on entry:
 *
 *   rax  system call number
 *   rdi  arg0
 *   rsi  arg1
 *   rdx  arg2
 *   rcx  arg3
 *   r8   arg4
 *   r9   arg5
 *
 * The current user stack pointer is saved in thread-local storage before
 * loading the kernel stack. General-purpose registers are then pushed to
 * form a struct pt_regs, whose address is passed to do_syscall().
 *
 *               Userspace                      Kernel
 *        +-------------------+         +-------------------+
 *        | user stack        |         | kernel stack      |
 *        |                   |         |                   |
 *        |  syscall(...)     |         |                   |
 *        +---------+---------+         +---------+---------+
 *                  |                             ^
 *                  | syscall                     |
 *                  v                             |
 *      save user rsp in TLS                      |
 *                  |                             |
 *                  +-----------------------------+
 *                  |
 *                  v
 *          load kernel rsp
 *                  |
 *                  v
 *          push pt_regs frame
 *                  |
 *                  v
 *          do_syscall(pt_regs *)
 *                  |
 *                  v
 *         restore registers
 *                  |
 *                  v
 *          restore user rsp
 *                  |
 *                  v
 *          return to userspace
 *
 * On return, the saved register state is restored from pt_regs, the user
 * stack pointer is reloaded from thread-local storage, and execution
 * resumes immediately after the original syscall instruction.
 */
	.text
	.global ientry_SYSCALL_64
	.type ientry_SYSCALL_64, @function
ientry_SYSCALL_64:
	movq	%rsp, %fs:saved_user_rsp@tpoff
	movq	%fs:current_kernel_rsp@tpoff, %rsp
	/* now on kernel stack, no register touched */

	pushq	%fs:saved_user_rsp@tpoff	/* sp */
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
	popq    %rcx        	/* RIP */
	popq    %r11        	/* RFLAGS */

	pushq   %r11
	popfq

	popq    %rsp        	/* User RSP */

	/* sysretq */
	jmp	*%rcx



	.global __kswitch_to_asm
	.type   __kswitch_to_asm, @function

/*
 * %rdi: prev task
 * %rsi: next task
 */
__kswitch_to_asm:
	pushq	%rbp
	pushq	%rbx
	pushq	%r12
	pushq	%r13
	pushq	%r14
	pushq	%r15

	/* prev->thread.sp.  */
	movq	%rsp, (%rdi)
	movq	(%rsi), %rsp 	/* rsp = next->thread.sp */

	popq	%r15
	popq	%r14
	popq	%r13
	popq	%r12
	popq	%rbx
	popq	%rbp

	jmp __kswitch_to

/*
 * A newly cloned task immediately switches it's context into this address.
 *
 * rax: prev task we switched from
 * rbx: kernel thread func (NULL for user thread)
 * r12: kernel thread arg
 */
	.global ret_from_fork_asm
	.type   ret_from_fork_asm, @function
ret_from_fork_asm:
	movq	%rax, %rdi	/* prev */
	movq	%rsp, %rdi	/* regs */
	movq	%rbx, %rdx	/* fn */
	movq	%r12, %rcx	/* fn_arg */
	call	ret_from_fork

	jmp iexit_SYSCALL_64
