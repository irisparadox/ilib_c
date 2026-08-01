#include <kinclude/kerrno.h>
#include <kinclude/syscall.h>

static long (*syscall_table[NR_SYSCALLS])(struct ipt_regs *) = {
	[SYS_FORK]  = isys_fork,
	[SYS_EXIT]  = isys_exit,
	[SYS_WAIT4] = isys_wait4,
	[SYS_YIELD] = isys_sched_yield,
};


long ido_syscall_64(struct ipt_regs *regs)
{
	unsigned long nr = regs->orig_ax;

	if (nr >= NR_SYSCALLS || !syscall_table[nr]) {
		regs->ax = -ENOSYS;
		return -ENOSYS;
	}

	regs->ax = syscall_table[nr](regs);
	return regs->ax;
}
