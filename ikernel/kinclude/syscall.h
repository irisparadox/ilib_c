#ifndef SYSCALL_H_
#define SYSCALL_H_

#include <kinclude/ktid.h>
#include <asm/iptrace.h>

#define SYS_CLONE3	0
#define SYS_EXIT	1
#define SYS_WAIT3	2
#define SYS_YIELD	3
#define SYS_GETTID	4

#define NR_SYSCALLS	5

#define __SC_DECL(t, a)	t a
#define __SC_CAST(t, a)	(t)a

#define __MAP0(m,...)
#define __MAP1(m,t,a,...) m(t,a)
#define __MAP2(m,t,a,...) m(t,a), __MAP1(m,__VA_ARGS__)
#define __MAP3(m,t,a,...) m(t,a), __MAP2(m,__VA_ARGS__)
#define __MAP4(m,t,a,...) m(t,a), __MAP3(m,__VA_ARGS__)
#define __MAP5(m,t,a,...) m(t,a), __MAP4(m,__VA_ARGS__)
#define __MAP6(m,t,a,...) m(t,a), __MAP5(m,__VA_ARGS__)
#define __MAP(n,...) __MAP##n(__VA_ARGS__)

#define __SC_TYPE(t, a)	t

#define SC_X86_64_PT_ARG0(m, ...)
#define SC_X86_64_PT_ARG1(m, t1) \
	m(t1, regs->di)
#define SC_X86_64_PT_ARG2(m, t1, t2) \
	SC_X86_64_PT_ARG1(m, t1), m(t2, regs->si)
#define SC_X86_64_PT_ARG3(m, t1, t2, t3) \
	SC_X86_64_PT_ARG2(m, t1, t2), m(t3, regs->dx)
#define SC_X86_64_PT_ARG4(m, t1, t2, t3, t4) \
	SC_X86_64_PT_ARG3(m, t1, t2, t3), m(t4, regs->r10)
#define SC_X86_64_PT_ARG5(m, t1, t2, t3, t4, t5) \
	SC_X86_64_PT_ARG4(m, t1, t2, t3, t4), m(t5, regs->r8)
#define SC_X86_64_PT_ARG6(m, t1, t2, t3, t4, t5, t6) \
	SC_X86_64_PT_ARG5(m, t1, t2, t3, t4, t5), m(t6, regs->r9)

#define SC_X86_64_PT_ARGS(x, m, ...) SC_X86_64_PT_ARG##x(m, __VA_ARGS__)

#define SC_X86_64_REGS_TO_ARGS(x, ...) \
	SC_X86_64_PT_ARGS(x, __SC_CAST, __MAP(x, __SC_TYPE, __VA_ARGS__))

#define __SYSCALL_DEFINEx(x, name, ...)					\
	static inline long __do_isys##name(__MAP(x,__SC_DECL, ##__VA_ARGS__));\
	long isys##name(struct ipt_regs *regs)					\
	{									\
		return __do_isys##name(SC_X86_64_REGS_TO_ARGS(x, ##__VA_ARGS__));\
	}									\
	static inline long __do_isys##name(__MAP(x,__SC_DECL, ##__VA_ARGS__))

#define SYSCALL_DEFINEx(x, name, ...)	__SYSCALL_DEFINEx(x, name, ##__VA_ARGS__)

#define SYSCALL_DEFINE0(name)		SYSCALL_DEFINEx(0, _##name)
#define SYSCALL_DEFINE1(name, ...)	SYSCALL_DEFINEx(1, _##name, __VA_ARGS__)
#define SYSCALL_DEFINE2(name, ...)	SYSCALL_DEFINEx(2, _##name, __VA_ARGS__)
#define SYSCALL_DEFINE3(name, ...)	SYSCALL_DEFINEx(3, _##name, __VA_ARGS__)
#define SYSCALL_DEFINE4(name, ...)	SYSCALL_DEFINEx(4, _##name, __VA_ARGS__)
#define SYSCALL_DEFINE5(name, ...)	SYSCALL_DEFINEx(5, _##name, __VA_ARGS__)
#define SYSCALL_DEFINE6(name, ...)	SYSCALL_DEFINEx(6, _##name, __VA_ARGS__)

extern long ido_syscall_64(struct ipt_regs *regs);

extern long isys_clone3(struct ipt_regs *regs);
extern long isys_exit(struct ipt_regs *regs);
extern long isys_wait3(struct ipt_regs *regs);
extern long isys_sched_yield(struct ipt_regs *regs);
extern long isys_gettid(struct ipt_regs *regs);

extern long isyscall(long nr, long a1, long a2, long a3, long a4, long a5);

#endif // SYSCALL_H_
