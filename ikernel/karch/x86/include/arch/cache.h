#ifndef CACHE_H_
#define CACHE_H_

#define L1_CACHE_SHIFT		(6)
#define L1_CACHE_BYTES		(1 << L1_CACHE_SHIFT)

#define __i__aligned(x)		__attribute__((aligned(x)))
#define __i__cacheline_aligned	__i__aligned(L1_CACHE_BYTES)

#endif // CACHE_H_
