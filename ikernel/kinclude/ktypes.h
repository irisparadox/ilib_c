#ifndef KTYPES_H_
#define KTYPES_H_

typedef unsigned char	u8;
typedef signed   char	s8;

typedef unsigned short	u16;
typedef signed   short	s16;

typedef unsigned long	u64;
typedef signed   long	s64;

typedef unsigned long	kuptr;

typedef int		__ikernel_tid;

#ifndef bool
#define bool	_Bool
#define true	1
#define false	0
#endif

#ifndef NULL
#define NULL	((void *)0)
#endif  /* NULL */

#if defined(__GNUC__) || defined(__clang__)
#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)
#else
#define likely(x)	(x)
#define unlikely(x)	(x)
#endif /* defined(__GNUC__) || defined(__clang__) */

typedef volatile long	__katomic_long_t;

#define __katomic_long_and(val, var)		__sync_fetch_and_and((var), (val))
#define __katomic_long_andnot(val, var)		__sync_fetch_and_and((var), ~(val))
#define __katomic_long_or(val, var)		__sync_fetch_and_or((var), (val))

#endif // KTYPES_H_
