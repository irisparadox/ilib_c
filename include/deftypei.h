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

#if defined(__GNUC__) || defined(__clang__)
#define ILIB_OFFSETOF(type, member) \
	__builtin_offsetof(type, member)
#else
#define ILIB_OFFSETOF(type, member) \
	((ilib_size_t)&(((type *)0)->member))
#endif /* defined(__GNUC__) || defined(__clang__) */

#if defined(__GNUC__) || defined(__clang__)
#define ILIB_ALIGN(n) __attribute__((aligned(n)))
#else
#error "Unsupported compiler."
#endif

#endif /* DEFTYPEI_H */
