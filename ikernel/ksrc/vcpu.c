#include <compiler_iattr.h>
#include <kinclude/vcpu.h>
#include <pthread.h>

static pthread_key_t tls_cpu;

struct ivcpu *cpu_self(void)
{
	return pthread_getspecific(tls_cpu);
}

struct ikern *ikern_self(void)
{
	return cpu_self()->kern;
}

struct rqi *cpu_rq(void)
{
	return cpu_self()->rq;
}

bool cpu_is_online(struct ivcpu *cpu)
{
	return IREAD_ONCE(unsigned, cpu->vstate) & CPU_ONLINE;
}

bool cpu_is_offline(struct ivcpu *cpu)
{
	return !cpu_is_online(cpu);
}

void cpu_submit_online(struct ivcpu *cpu)
{
	pthread_mutex_lock(&cpu->cpu_lock);
	cpu->flags = CPU_ONLINE;
	pthread_cond_signal(&cpu->cpu_signal);
	pthread_mutex_unlock(&cpu->cpu_lock);
}

void cpu_submit_offline(struct ivcpu *cpu)
{
	pthread_mutex_lock(&cpu->cpu_lock);
	cpu->flags = CPU_OFFLINE;
	pthread_cond_signal(&cpu->cpu_signal);
	pthread_mutex_unlock(&cpu->cpu_lock);
}
