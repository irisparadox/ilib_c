#ifndef VCPU_H_
#define VCPU_H_

#include <pthread.h>
#include <compiler_iattr.h>
#include <kinclude/kconf.h>
#include <kinclude/ktypes.h>

#define CPU_OFFLINE	0
#define CPU_ONLINE	1

#define KSELECT_ALL_CORES ((u64)-1)

#define CPU_F_BRINGUP	(1u << 0)
#define CPU_F_WAITJOIN	(1u << 1)
#define CPU_F_DELEGATE	(1u << 2)

struct ikern;
struct rqi;

struct ivcpu_array {
	unsigned int		ncpu;
	unsigned int		online;

	struct ivcpu		*ventries[NR_VCPU];
};

struct ivcpu {
	int			cpu_id;

	unsigned int		flags;
	unsigned int		vstate;

	struct rqi		*rq;

	pthread_t		vthread;
	pthread_mutex_t		cpu_lock;
	pthread_cond_t		cpu_signal;

	struct ikern		*kern;

#if KCONF_VCPU_STATS == 1
	u64			vruntime;
	u64			nr_migrate;
	u64			nr_ran;
#endif /* KCONF_VCPU_STATS */
};

extern struct ivcpu	*cpu_self(void);
extern struct ikern	*ikern_self(void);
extern struct rqi	*cpu_rq(void);
extern bool		cpu_is_online(struct ivcpu *cpu);
extern bool		cpu_is_offline(struct ivcpu *cpu);

extern void		cpu_submit_online(struct ivcpu *cpu);
extern void		cpu_submit_offline(struct ivcpu *cpu);

void __inoreturn	cpu_startup_entry(void);

#endif // VCPU_H_
