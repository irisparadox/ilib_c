#ifndef COMPILER_IATTR_H_
#define COMPILER_IATTR_H_

#include <deftypei.h>

#define __isection(section)	__attribute__((__section__(section)))
#define __ialigned(x)      	__attribute__((__aligned__(x)))
#define __inoreturn        	__attribute__((__noreturn__))
#define __ialways_inline   	inline __attribute__((__always_inline__))

#define IREAD_ONCE(type, x) (*(const volatile type *)&(x))
#define IWRITE_ONCE(type, x, val) do { *(volatile type *)&(x) = (val); } while (0)

#define atomic_landnot(var, val) __sync_fetch_and_and(&(var), ~(val))
#define atomic_lor(var, val) __sync_fetch_and_or(&(var), (val))

#define asm __asm__
#define volatile __volatile__

#if defined(__x86_64__)
static __ialways_inline bool arch_test_bit(long nr, const volatile unsigned long *addr)
{
	bool oldbit;

	asm volatile(
		"bt %2,%1\n\t"
		"sbb %0,%0"
		: "=r" (oldbit)
		: "m"  (*(unsigned long *)addr), "Ir" (nr)
		: "cc"
	 );

	return oldbit;
}
#endif /* defined(__x86_64__) */

#endif // COMPILER_IATTR_H_
