#ifndef IPTRACE_H_
#define IPTRACE_H_

struct ipt_regs {
	unsigned long	r15;
	unsigned long	r14;
	unsigned long	r13;
	unsigned long	r12;
	unsigned long	bp;
	unsigned long	bx;
	/* general purpose, caller-clobbered */
	unsigned long	r11;
	unsigned long	r10;
	unsigned long	r9;
	unsigned long	r8;
	unsigned long	ax;
	unsigned long	cx;
	unsigned long	dx;
	unsigned long	si;
	unsigned long	di;
	unsigned long	orig_ax;
	unsigned long	ip;
	unsigned long	flags;
	unsigned long	sp;
};

#define task_pt_regs(p)						\
	((struct ipt_regs *)((char *)p->stack + KSTACK_SIZE) - 1)

#define current_pt_regs()	task_pt_regs(current)

#endif // IPTRACE_H_
