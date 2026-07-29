#ifndef COMPILER_IATTR_H_
#define COMPILER_IATTR_H_

#define __isection(section) __attribute__((__section__(section)))
#define __ialigned(x)       __attribute__((__aligned__(x)))

#define IREAD_ONCE(type, x) (*(const volatile type *)&(x))
#define IWRITE_ONCE(type, x, val) do { *(volatile type *)&(x) = (val); } while (0)

#define atomic_landnot(var, val) __sync_fetch_and_and(&(var), ~(val))
#define atomic_lor(var, val) __sync_fetch_and_or(&(var), (val))

#endif // COMPILER_IATTR_H_
