#ifndef KFUN_H_
#define KFUN_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <deftypei.h>
#include <compiler_iattr.h>

#define BUG() do {						\
	fprintf(stderr, "BUG: failure at %s:%d/%s()!\n",	\
		__FILE__, __LINE__, __func__);			\
	abort();						\
} while (0)

#define BUG_ON(condition) do { if (iunlikely(condition)) { BUG(); } } while (0)

static inline void barrier(void)
{
	asm volatile("" : : : "memory");
}

#define container_of(ptr, type, member)					\
	((type *)((char *)(ptr) - (unsigned long)&(((type *)0)->member)))

static inline void mutex_guard_release(pthread_mutex_t **lock)
{
	if (*lock)
		pthread_mutex_unlock(*lock);
}

#define scoped_mutex(lockp)								\
for (pthread_mutex_t *_g __attribute__((cleanup(mutex_guard_release))) = (lockp),	\
	*_once = (pthread_mutex_lock(_g), NULL);					\
	_once == NULL; _once = (void *)1)

#endif // KFUN_H_
