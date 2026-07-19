#include <x86-64/i_context_asm.h>
	.text
	.global __i___start_context
	.type __i___start_context, @function
__i___start_context:
	/* This removes the parameters passed to the function given to
	   'imakecontext' from the stack. RBX containes the address on
	   the stack pointer for the next context.  */
	movq	%rbx, %rsp

	/* Don't use pop here so that stack is aligned to 16 bytes.  */
	movq	(%rsp), %rdi		/* This is the next context.  */
	testq	%rdi, %rdi
	je	2f			/* If it's zero exit.  */

	call isetcontext
	/* If this returns (which can happen if the syscall fails) we'll
	   exit the program with the return error value (-1).  */
	movq	%rax,%rdi

2:
	call	exit
	/* The 'exit' call should never return. In case it does cause
	   the process to terminate.  */
	hlt
