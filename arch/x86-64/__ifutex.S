	.text
	.global futex_wait_zero
	# int futex_wait_zero(u32 *addr);
	# Blocks iff *addr == 0 at the moment the kernel checks it.
futex_wait_zero:
.Lretry:
	xor	%r10, %r10	# timeout = NULL
	xor	%r8,  %r8	# uaddr2 (unused)
	xor	%r9,  %r9	# val3   (unused)
	mov	$0,   %edx	# val: only wait if *addr still == 0
	mov	$0,   %esi	# FUTEX_WAIT
	mov	$202, %eax	# __NR_futex
	syscall
	cmp	$0,   %eax	# woke normally
	jge	.Ldone
	cmp	$-4,  %eax	# -EINTR
	je	.Lretry
	cmp	$-11, %eax	# -EAGAIN: value changed before we blocked, not an error
	je	.Ldone
.Ldone:
	ret

	.global futex_wake_one
	# int futex_wake_one(u32 *addr);
futex_wake_one:
	mov	$1,   %esi	# FUTEX_WAKE
	mov	$1,   %edx	# wake 1 waiter
	mov	$202, %eax	# __NR_futex
	syscall
	ret
