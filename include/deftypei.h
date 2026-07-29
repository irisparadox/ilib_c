#ifndef DEFTYPEI_H
#define DEFTYPEI_H

typedef signed char    ilib_int8_t;
typedef unsigned char  ilib_uint8_t;

typedef signed short   ilib_int16_t;
typedef unsigned short ilib_uint16_t;

typedef signed int     ilib_int32_t;
typedef unsigned int   ilib_uint32_t;

typedef signed long    ilib_int64_t;
typedef unsigned long  ilib_uint64_t;

typedef unsigned long  ilib_size_t;
typedef signed long    ilib_ssize_t;

typedef unsigned char  ilib_byte_t;
typedef signed long    ilib_intptr_t;
typedef unsigned long  ilib_uintptr_t;

#ifndef bool
#define bool  _Bool
#define true  1
#define false 0
#endif

#if defined(__GNUC__) || defined(__clang__)
#define ILIB_OFFSETOF(type, member) \
	__builtin_offsetof(type, member)
#define ILIB_ALIGN(n) __attribute__((aligned(n)))
#define ILIB_HIDDEN __attribute__((visibility("hidden")))
#else
#define ILIB_OFFSETOF(type, member) \
	((ilib_size_t)&(((type *)0)->member))
#define ILIB_ALIGN
#define ILIB_HIDDEN
#endif /* defined(__GNUC__) || defined(__clang__) */

#ifndef NULL
#define NULL ((void *)0)
#endif

#if defined(__GNUC__) || defined(__clang__)
#define ilikely(x)	__builtin_expect(!!(x), 1)
#define iunlikely(x)	__builtin_expect(!!(x), 0)
#else
#define ilikely(x)	(x)
#define iunlikely(x)	(x)
#endif /* defined(__GNUC__) || defined(__clang__) */

#define CHAR_BIT 8

#define BITS_PER_BYTE CHAR_BIT
#define BITS_PER_TYPE(type) (sizeof(type) * BITS_PER_BYTE)
#define __ILIB_DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define BITS_TO_LONGS(nr) \
	__ILIB_DIV_ROUND_UP(nr, BITS_PER_TYPE(long))
#define DECLARE_BITMAP(name, bits) \
	unsigned long name[BITS_TO_LONGS(bits)]

#define bitmap_test(bmp, bit) bmp[bit / BITS_PER_TYPE(long)] & \
	(1UL << (bit % BITS_PER_TYPE(long)))

#define bitmap_set(bmp, bit) bmp[bit / BITS_PER_TYPE(long)] |= \
	(1UL << (bit % BITS_PER_TYPE(long)))

#define bitmap_clear(bmp, bit) bmp[bit / BITS_PER_TYPE(long)] &= \
	~(1UL << (bit % BITS_PER_TYPE(long)))

#if defined(__GNUC__) || defined(__clang__)
#define __imust_be_array(arr) \
(sizeof(char[1 - 2 * __builtin_types_compatible_p(__typeof__(arr), __typeof__(&(arr)[0]))]) - 1)

#define IARRAY_SIZE(arr) \
(sizeof(arr) / sizeof((arr)[0]) + __imust_be_array(arr))
#else
#define IARRAY_SIZE(arr) \
(sizeof(arr) / sizeof((arr)[0]))
#endif /* defined(__GNUC__) || defined(__clang__) */

#endif /* DEFTYPEI_H */
