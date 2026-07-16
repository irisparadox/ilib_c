#ifndef IHSTMAP_H_
#define IHSTMAP_H_

/* IHSTMAP_F_ITERATORS
 * If defined as 1 iterators will be enabled.
 * By default this macro is defined as 1.
 */
#ifndef IHSTMAP_F_ITERATORS
#define IHSTMAP_F_ITERATORS 1
#endif  /* IHSTMAP_F_ITERATORS */

/* IHSTMAP_F_DEBUG
 * Enable debug tools
 * By default this macro is defined as 0
 */
#ifndef IHSTMAP_F_DEBUG
#define IHSTMAP_F_DEBUG 0
#endif  /* IHSTMAP_F_DEBUG */

/* IHSTMAP_F_SIMD
 * Enable Simple Instruction - Multiple Data support.
 * This enables low-level optimizations on CPUs
 * that support vectorization.
 * By default this feature is defined as 1 (enabled).
 */
#ifndef IHSTMAP_F_SIMD
#define IHSTMAP_F_SIMD 1
#endif  /* IHSTMAP_F_SIMD */

/* IHSTMAP_INITIAL_CAPACITY
 * Change the initial capacity to something else.
 * The value has to be a power of two, or else it won't compile.
 */
#ifndef IHSTMAP_INITIAL_CAPACITY
#define IHSTMAP_INITIAL_CAPACITY 16
#elif ((IHSTMAP_INITIAL_CAPACITY) == 0 || \
       ((IHSTMAP_INITIAL_CAPACITY) & ((IHSTMAP_INITIAL_CAPACITY) - 1)) != 0)
#error "The initial capacity has to be a power of 2"
#endif  /* IHSTMAP_INITIAL_CAPACITY */

/* IHSTMAP_MAX_LOAD
 * Maximum load factor.
 * Default is 87.5%
 */
#ifndef IHSTMAP_MAX_LOAD
#define IHSTMAP_MAX_LOAD 875
#endif  /* IHSTMAP_MAX_LOAD */

#include "deftypei.h"



#define IHSTMAP_OK        (0)
#define IHSTMAP_ERRINVAL (-1)
#define IHSTMAP_ERRNOMEM (-2)
#define IHSTMAP_ERREXIST (-3)
#define IHSTMAP_ERRNTFND (-4)
#define IHSTMAP_ERRFULL  (-5)



typedef struct ihstmap_al ihstmap_al_t;

typedef struct ihstmap_al_ops {
	void *(*alloc)(ilib_size_t size, void *usrdata);
	void *(*realloc)(void *ptr, ilib_size_t size, void *usrdata);
	void (*free)(void *ptr, void *usrdata);
} ihstmap_al_ops_t;

typedef ilib_uint64_t (*ihstmap_hash_fn)(const void *key, void *usrdata);
typedef int           (*ihstmap_eq_fn)(const void *a, const void *b);




typedef struct ihstmap {
	ilib_size_t size;
	ilib_size_t capacity;

	void       *entries;
	void       *control;

	const struct ihstmap_al *al;
} ihstmap_t;

typedef struct ihstmap_entry {
	const void *key;
	void *value;
} ihstmap_entry_t;



#if IHSTMAP_F_ITERATORS == 1
typedef struct ihstmap_it ihstmap_it_t;

struct ihstmap_it {
	ihstmap_t   *map;
	ilib_size_t  idx;
};

#endif /* IHSTMAP_F_ITERATORS == 1 */



/* IHSTMAP_PREHASH
 *     The hash value is provided by the caller.
 *     Requires an additional ilib_u64_t argument.
 *
 * IHSTMAP_NOGROW
 *     Prevent the hashmap from increasing its capacity.
 *     The operation fails if additional space is required.
 *
 * IHSTMAP_ALTMODE
 *     Enables mode field which makes insert mode differ
 *     from default behaviour.
 */
#define IHSTMAP_PREHASH (1 << 0)
#define IHSTMAP_NOGROW  (1 << 1)
#define IHSTMAP_ALTMODE (1 << 2)

/* Insert modes */
#define IHSTMAP_ISTREP   (1 << 0)
#define IHSTMAP_ISTNOREP (1 << 1)
#define IHSTMAP_ISTRETEX (1 << 2)



int ihstmap_construct(ihstmap_t *map, const ihstmap_al_t *al, ihstmap_hash_fn hash, ihstmap_eq_fn eq);
int ihstmap_destroy(ihstmap_t *map);

void *ihstmap_get(const ihstmap_t *map, const void *key);

int ihstmap_insert(ihstmap_t *map, const void *key, void *val, ilib_uint32_t mode, ...);
int ihstmap_remove(ihstmap_t *map, const void *key);
int ihstmap_reserve(ihstmap_t *map, ilib_size_t capacity);
int ihstmap_clear(ihstmap_t *map);

#if IHSTMAP_F_ITERATORS == 1
ihstmap_it_t ihstmap_begin(const ihstmap_t *map);
ihstmap_it_t ihstmap_end(const ihstmap_t *map);

ihstmap_it_t ihstmap_find(const ihstmap_t *map, const void *key, ihstmap_entry_t *entry);
ihstmap_it_t ihstmap_next(ihstmap_it_t *it);
#endif /* IHSTMAP_F_ITERATORS == 1 */

ilib_size_t ihstmap_size(const ihstmap_t *map);
ilib_size_t ihstmap_capacity(const ihstmap_t *map);
int         ihstmap_empty(const ihstmap_t *map);



#endif // IHSTMAP_H_

#ifdef IHSTMAP_IMPLEMENTATION
#ifndef __I_IHSTMAP_IMPL
#define __I_IHSTMAP_IMPL

struct ihstmap_al {
	const ihstmap_al_ops_t *ops;
	void *usrdata;
};

#if IHSTMAP_F_ITERATORS == 1
#endif /* IHSTMAP_F_ITERATORS == 1 */

#endif /* __I_IHSTMAP_IMPL */
#endif /* IHSTMAP_IMPLEMENTATION */
