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

#define IHSTMAP_2POWER_CHECK(x) ((x) == 0 || \
       ((x) & ((x) - 1)) != 0)

/* IHSTMAP_INITIAL_CAPACITY
 * Change the initial capacity to something else.
 * The value has to be a power of two, or else it won't compile.
 */
#ifndef IHSTMAP_INITIAL_CAPACITY
#define IHSTMAP_INITIAL_CAPACITY 16
#elif   IHSTMAP_2POWER_CHECK(IHSTMAP_INITIAL_CAPACITY)
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

typedef struct {
	ihstmap_hash_fn hash;
	ihstmap_eq_fn eq;
	void *usrdata;
} ihstmap_hash_t;



typedef struct ihstmap {
	ilib_size_t size;
	ilib_size_t capacity;

	void       *entries;
	void       *control;

	const ihstmap_hash_t    *h;
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



typedef ilib_uint32_t ihm_flag_t;
typedef ilib_uint32_t ihm_mode_t;

/* IHSTMAP_PREHASH
 *     The hash value is provided by the caller.
 *     Requires an additional ilib_uint64_t argument.
 *
 * IHSTMAP_NOGROW
 *     Prevent the hashmap from increasing its capacity.
 *     The operation fails if additional space is required.
 *     Takes precedence if combined with IHSTMAP_FORCEGROW.
 *
 * IHSTMAP_ALTMODE
 *     Enables mode field which makes insert mode differ
 *     from default behaviour.
 *
 * IHSTMAP_FRCGROW
 *     Forces the table to grow before placing a new entry,
 *     even if the current load factor would not require it.
 *     Has no effect on keys that already exist (replace never
 *     triggers growth).
 *
 * IHSTMAP_RAWCAP
 *     Changes the growth trigger to compare size against raw
 *     capacity instead of IHSTMAP_MAX_LOAD, allowing the table
 *     to fill past the normal 87.5% threshold before growing.
 *     Degrades average probe length as capacity is approached.
 */
#define IHSTMAP_PREHASH (1 << 0)
#define IHSTMAP_NOGROW  (1 << 1)
#define IHSTMAP_ALTMODE (1 << 2)
#define IHSTMAP_FRCGROW (1 << 3)
#define IHSTMAP_RAWCAP  (1 << 4)

/* Insert mode flags for use with IHSTMAP_ALTMODE.
 * See ihstmap_insert's declaration comment for read order and semantics. */
#define IHSTMAP_ISTREP   (1 << 0)
#define IHSTMAP_ISTNOREP (1 << 1)
#define IHSTMAP_ISTRETEX (1 << 2)



int ihstmap_construct(ihstmap_t *map, const ihstmap_al_t *al, const ihstmap_hash_t *hash);
int ihstmap_destroy(ihstmap_t *map);

void *ihstmap_get(const ihstmap_t *map, const void *key);

int ihstmap_insert(ihstmap_t *map, const void *key, void *val, ihm_flag_t flags, ...);
int ihstmap_remove(ihstmap_t *map, const void *key);
int ihstmap_reserve(ihstmap_t *map, ilib_size_t capacity);
int ihstmap_clear(ihstmap_t *map);

#if IHSTMAP_F_ITERATORS == 1
ihstmap_it_t    ihstmap_begin(const ihstmap_t *map);
ihstmap_it_t    ihstmap_end(const ihstmap_t *map);

ihstmap_it_t    ihstmap_find(const ihstmap_t *map, const void *key, ihstmap_entry_t *entry);
ihstmap_it_t    ihstmap_next(ihstmap_it_t *it);
ihstmap_entry_t ihstmap_it_get(const ihstmap_it_t *it);
#endif /* IHSTMAP_F_ITERATORS == 1 */

ilib_size_t ihstmap_size(const ihstmap_t *map);
ilib_size_t ihstmap_capacity(const ihstmap_t *map);
int         ihstmap_empty(const ihstmap_t *map);

/* TODO: str, u32, u64 */
extern const ihstmap_hash_t ihstmap_ptr_hash;
extern const ihstmap_hash_t ihstmap_str_hash;
extern const ihstmap_hash_t ihstmap_u32_hash;
extern const ihstmap_hash_t ihstmap_u64_hash;

#endif // IHSTMAP_H_

//#define IHSTMAP_IMPLEMENTATION
#ifdef IHSTMAP_IMPLEMENTATION
#ifndef I_IHSTMAP_IMPL_
#define I_IHSTMAP_IMPL_

#if !((defined(IHM_MALLOC) == defined(IHM_FREE)) && (defined(IHM_FREE) == defined(IHM_REALLOC)))
#error "You must either use the default allocator or provide all of: IHM_MALLOC, IHM_REALLOC and IHM_FREE"
#endif

#if !defined(IHM_MALLOC) || !defined(IHM_FREE) || !defined(IHM_REALLOC)

#include <stdlib.h>

#ifndef IHM_MALLOC
#define IHM_MALLOC malloc
#endif /* IHM_MALLOC */

#ifndef IHM_FREE
#define IHM_FREE free
#endif /* IHM_FREE */

#ifndef IHM_REALLOC
#define IHM_REALLOC realloc
#endif /* IHM_REALLOC */

#endif /* !defined(IHM_MALLOC) || !defined(IHM_FREE) || !defined(IHM_REALLOC) */

#if !(defined(IHM_MEMSET) == defined(IHM_MEMCPY))
#error "You must either use the default mem ops or provide IHM_MEMSET and IHM_MEMCPY"
#endif

#if !defined(IHM_MEMSET) || !defined(IHM_MEMCPY)

#include <string.h>

#ifndef IHM_MEMSET
#define IHM_MEMSET memset
#endif /* IHM_MEMSET */

#ifndef IHM_MEMCPY
#define IHM_MEMCPY memcpy
#endif /* IHM_MEMCPY */

#endif /* !defined(IHM_MEMSET) || !defined(IHM_MEMCPY) */

#include <stdarg.h>
#include <stdio.h>

struct ihstmap_al {
	const ihstmap_al_ops_t *ops;
	void *usrdata;
};

typedef ilib_uint8_t ihstmap_ctrl_t;

#define I_IHSTMAP_CTRL_EMPTY    ((ihstmap_ctrl_t)0x80)
#define I_IHSTMAP_CTRL_DELETED  ((ihstmap_ctrl_t)0xFE)
#define I_IHSTMAP_CTRL_SENTINEL ((ihstmap_ctrl_t)0xFF) /* reserved */

#define I_IHSTMAP_CTRL_IS_EMPTY(c)   ((c) == I_IHSTMAP_CTRL_EMPTY)
#define I_IHSTMAP_CTRL_IS_DELETED(c) ((c) == I_IHSTMAP_CTRL_DELETED)
#define I_IHSTMAP_CTRL_IS_FULL(c)    (((c) & 0x80) == 0)

/* FOOTPRINT EXTRACTION METHODS */

static inline ilib_uint64_t ihm__h1(ilib_uint64_t h)
{
	return h >> 7;
}

static inline ihstmap_ctrl_t ihm__h2(ilib_uint64_t h)
{
	return (ihstmap_ctrl_t)(h & 0x7f);
}


typedef struct ihstmap__probe {
	ilib_size_t idx;
	ilib_size_t mask;
	ilib_size_t step;
	ilib_size_t count;
} ihstmap__probe_t;

static inline ihstmap__probe_t ihm__probe_start(const ihstmap_t *map, ilib_uint64_t hash)
{
	ihstmap__probe_t p;

	p.mask  = map->capacity - 1;
	p.idx   = ihm__h1(hash) & p.mask;
	p.step  = 0;
	p.count = 0;

	return p;
}

static inline void ihm__probe_next(ihstmap__probe_t *p)
{
	p->step++;
	p->count++;
	p->idx = (p->idx + p->step) & p->mask;
}

static inline int ihm__probe_done(const ihstmap__probe_t *p, ilib_size_t cap)
{
	return p->count >= cap;
}

#define I_MAPALLOC(m, size) \
	((m)->al->ops->alloc((size), (m)->al->usrdata))

#define I_MAPFREE(m, ptr) \
	((m)->al->ops->free((ptr), (m)->al->usrdata))

/* Default hashing and key equalization callbacks. If the user doesn't
 * provide one, the constructor will automatically assign these.
 *
 * The hashing is done on top of pointers.
 */
static ilib_uint64_t ihstmap__hash_ptr(const void *key, void *usrdata)
{
	(void)usrdata;
	return (ilib_uint64_t)(ilib_uintptr_t)key;
}

static int ihstmap__eq_ptr(const void *a, const void *b)
{
	return a == b;
}

const ihstmap_hash_t ihstmap_ptr_hash = {
	.hash = ihstmap__hash_ptr,
	.eq = ihstmap__eq_ptr,
	.usrdata = NULL
};

/* PRIVATE METHODS */

static void *ihstmap__malloc(ilib_size_t size, void *usrdata)
{
	(void)usrdata;
	return IHM_MALLOC(size);
}

static void *ihstmap__realloc(void *ptr, ilib_size_t size, void *usrdata)
{
	(void)usrdata;
	return IHM_REALLOC(ptr, size);
}

static void ihstmap__free(void *ptr, void *usrdata)
{
	(void)usrdata;
	IHM_FREE(ptr);
}

static const ihstmap_al_ops_t i_ihstmap_default_al_ops = {
	.alloc   = ihstmap__malloc,
	.realloc = ihstmap__realloc,
	.free    = ihstmap__free
};

static const ihstmap_al_t i_ihstmap_default_al = {
	.ops = &i_ihstmap_default_al_ops,
	.usrdata = NULL
};

static int ihstmap__ctrl_alloc(ihstmap_t *map, ilib_size_t capacity)
{
	ihstmap_ctrl_t *ctrl;

	ctrl = I_MAPALLOC(map, capacity * sizeof(ihstmap_ctrl_t));
	if (ctrl == NULL)
		return IHSTMAP_ERRNOMEM;

	map->control = ctrl;
	return IHSTMAP_OK;
}

static void ihstmap__ctrl_init(ihstmap_t *map, ilib_size_t capacity)
{
	ihstmap_ctrl_t *ctrl = (ihstmap_ctrl_t *)map->control;

	IHM_MEMSET(ctrl, I_IHSTMAP_CTRL_EMPTY, capacity * sizeof(ihstmap_ctrl_t));
}

static int ihstmap__entries_alloc(ihstmap_t *map, ilib_size_t capacity)
{
	ihstmap_entry_t *entries;

	entries = I_MAPALLOC(map, capacity * sizeof(ihstmap_entry_t));
	if (entries == NULL)
		return IHSTMAP_ERRNOMEM;

	map->entries = entries;
	return IHSTMAP_OK;
}

static ilib_size_t ihm__next_pow2(ilib_size_t n)
{
	if (n <= 1)
		return 1;

	--n;

	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 3;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;

	if (sizeof(ilib_size_t) == 8)
		n |= n >> 32;

	return n + 1;
}

static void ihm__rehash_insert(ihstmap_ctrl_t *ctrl, ihstmap_entry_t *entries, ilib_size_t cap, const ihstmap_hash_t *h, const void *key, void *val)
{
	ilib_uint64_t hash;
	ilib_size_t   mask;
	ilib_size_t   idx;
	ilib_size_t   step;

	hash = h->hash(key, h->usrdata);
	mask = cap - 1;
	idx  = ihm__h1(hash) & mask;
	step = 0;

	while (!I_IHSTMAP_CTRL_IS_EMPTY(ctrl[idx])) {
		++step;
		idx = (idx + step) & mask;
	}

	ctrl[idx] = ihm__h2(hash);
	entries[idx].key   = key;
	entries[idx].value = val;
}

/* CONSTRUCTION AND DESTRUCTION */

int ihstmap_construct(ihstmap_t *map, const ihstmap_al_t *al, const ihstmap_hash_t *hash)
{
	int err;

	if (map == NULL) return IHSTMAP_ERRINVAL;

	map->size     = 0;
	map->capacity = IHSTMAP_INITIAL_CAPACITY;
	map->al       = (al != NULL) ? al : &i_ihstmap_default_al;
	map->h        = (hash != NULL) ? hash : &ihstmap_ptr_hash;

	if (map->h->hash == NULL || map->h->eq == NULL)
		return IHSTMAP_ERRINVAL;

	err = ihstmap__ctrl_alloc(map, map->capacity);
	if (err != IHSTMAP_OK) return err;

	ihstmap__ctrl_init(map, map->capacity);

	err = ihstmap__entries_alloc(map, map->capacity);
	if (err != IHSTMAP_OK) {
		I_MAPFREE(map, map->control);
		return err;
	}

	return IHSTMAP_OK;
}

int ihstmap_destroy(ihstmap_t *map)
{
	if (map == NULL)
		return IHSTMAP_ERRINVAL;

	if (map->control != NULL)
		I_MAPFREE(map, map->control);

	if (map->entries != NULL)
		I_MAPFREE(map, map->entries);

	map->control  = NULL;
	map->entries  = NULL;
	map->size     = 0;
	map->capacity = 0;

	return IHSTMAP_OK;
}

/* LOOKUP */

void *ihstmap_get(const ihstmap_t *map, const void *key)
{
	ilib_uint64_t     h;
	ihstmap_ctrl_t    c;
	ihstmap__probe_t  p;
	ihstmap_ctrl_t   *ctrl;
	ihstmap_entry_t  *entries;

	h       = map->h->hash(key, map->h->usrdata);
	ctrl    = (ihstmap_ctrl_t *)map->control;
	entries = (ihstmap_entry_t *)map->entries;
	p       = ihm__probe_start(map, h);

	while (!ihm__probe_done(&p, map->capacity)) {
		c = ctrl[p.idx];

		if (I_IHSTMAP_CTRL_IS_EMPTY(c))
			return NULL;

		if (I_IHSTMAP_CTRL_IS_FULL(c) &&
		    (ihstmap_ctrl_t)(c & 0x7f) == ihm__h2(h) &&
		    map->h->eq(entries[p.idx].key, key))
			return entries[p.idx].value;

		ihm__probe_next(&p);
	}

	return NULL;
}

/* INSERTION */
/* ihstmap_insert: insert or replace key/val.
 *
 * flags: IHSTMAP_ALTMODE    -> reads mode (ilib_uint32_t) next.
 *        IHSTMAP_PREHASH    -> reads hash (ilib_uint64_t) after mode.
 *        IHSTMAP_NOGROW     -> return IHSTMAP_ERRFULL instead of growing
 *                              (takes precedence over IHSTMAP_FORCEGROW).
 *        IHSTMAP_FRCGROW  -> grow before placing a new entry, even if
 *                              the load factor wouldn't require it.
 *        IHSTMAP_RAWCAP     -> grow trigger compares size against raw
 *                              capacity instead of IHSTMAP_MAX_LOAD.
 * Variadic order: mode, [existing (void**) if mode & ISTRETEX], hash.
 * These growth flags only apply when the key is new; a replace never
 * triggers growth.
 *
 * mode: ISTREP replaces on match (default if ALTMODE unset).
 *       ISTNOREP fails with ERREXIST on match instead.
 *       ISTRETEX (OR'd on either) writes old value to *existing.
 */
int ihstmap_insert(ihstmap_t *map, const void *key, void *val, ihm_flag_t flags, ...)
{
	ilib_uint64_t     h;
	ihstmap_ctrl_t    c;
	ihstmap__probe_t  p;
	ihstmap_ctrl_t   *ctrl;
	ihstmap_entry_t  *entries;
	ilib_size_t       tombidx;
	int               hastomb;
	int               err;

	va_list    ap;
	ihm_mode_t mode;
	void **existing;

	mode     = IHSTMAP_ISTREP;
	existing = NULL;

	va_start(ap, flags);

	if (flags & IHSTMAP_ALTMODE) {
		mode = va_arg(ap, ilib_uint32_t);
		if (mode & IHSTMAP_ISTRETEX)
			existing = va_arg(ap, void **);
	}

	if (flags & IHSTMAP_PREHASH)
		h = va_arg(ap, ilib_uint64_t);
	else
		h = map->h->hash(key, map->h->usrdata);

	va_end(ap);

 probe:
	ctrl    = (ihstmap_ctrl_t *)map->control;
	entries = (ihstmap_entry_t *)map->entries;
	hastomb = 0;
	tombidx = 0;

	p = ihm__probe_start(map, h);

	while (!ihm__probe_done(&p, map->capacity)) {
		c = ctrl[p.idx];

		if (I_IHSTMAP_CTRL_IS_EMPTY(c)) {
			ilib_size_t target = hastomb ? tombidx : p.idx;
			ilib_size_t thresh;
			int         shouldgrow;

			thresh = (flags & IHSTMAP_RAWCAP) ? map->capacity :
				 (map->capacity * IHSTMAP_MAX_LOAD) / 1000;

			shouldgrow = (map->size + 1 > thresh) || (flags & IHSTMAP_FRCGROW);

			if (shouldgrow) {
				if (flags & IHSTMAP_NOGROW)
					return IHSTMAP_ERRFULL;

				err = ihstmap_reserve(map, map->capacity << 1);
				if (err != IHSTMAP_OK) return err;

				flags &= ~IHSTMAP_FRCGROW;
				goto probe;
			}

			ctrl[target] = ihm__h2(h);
			entries[target].key   = key;
			entries[target].value = val;

			map->size++;

			return IHSTMAP_OK;
		}

		if (I_IHSTMAP_CTRL_IS_DELETED(c)) {
			if (!hastomb) {
				hastomb = 1;
				tombidx = p.idx;
			}
		} else if (I_IHSTMAP_CTRL_IS_FULL(c) &&
			   (ihstmap_ctrl_t)(c & 0x7f) == ihm__h2(h) &&
			   map->h->eq(entries[p.idx].key, key)) {

			if (existing != NULL)
				*existing = entries[p.idx].value;

			if (mode & IHSTMAP_ISTNOREP)
				return IHSTMAP_ERREXIST;

			entries[p.idx].value = val;
			return IHSTMAP_OK;
		}

		ihm__probe_next(&p);
	}

	return IHSTMAP_ERRFULL;
}

int ihstmap_remove(ihstmap_t *map, const void *key)
{
	ilib_uint64_t     h;
	ihstmap_ctrl_t    c;
	ihstmap__probe_t  p;
	ihstmap_ctrl_t   *ctrl;
	ihstmap_entry_t  *entries;

	ctrl    = (ihstmap_ctrl_t *)map->control;
	entries = (ihstmap_entry_t *)map->entries;

	h = map->h->hash(key, map->h->usrdata);
	p = ihm__probe_start(map, h);

	while (!ihm__probe_done(&p, map->capacity)) {
		c = ctrl[p.idx];

		if (I_IHSTMAP_CTRL_IS_EMPTY(c))
			return IHSTMAP_ERRNTFND;

		if (I_IHSTMAP_CTRL_IS_FULL(c) &&
		    (ihstmap_ctrl_t)(c & 0x7f) == ihm__h2(h) &&
		    map->h->eq(entries[p.idx].key, key)) {
			ctrl[p.idx] = I_IHSTMAP_CTRL_DELETED;
			map->size--;
			return IHSTMAP_OK;
		}

		ihm__probe_next(&p);
	}

	return IHSTMAP_ERRNTFND;
}

int ihstmap_reserve(ihstmap_t *map, ilib_size_t capacity)
{
	ilib_size_t needed;
	ilib_size_t newcap;
	ihstmap_ctrl_t  *new_ctrl;
	ihstmap_entry_t *new_entries;

	ilib_size_t      oldcap;
	ihstmap_ctrl_t  *old_ctrl;
	ihstmap_entry_t *old_entries;

	ilib_size_t i;

	if (map == NULL)
		return IHSTMAP_ERRINVAL;

	if (capacity <= map->capacity)
		return IHSTMAP_OK;

	needed = (capacity * 1000) / IHSTMAP_MAX_LOAD + 1;
	newcap = ihm__next_pow2(needed);

	new_ctrl = I_MAPALLOC(map, newcap * sizeof(ihstmap_ctrl_t));
	if (new_ctrl == NULL) {
		return IHSTMAP_ERRNOMEM;
	}

	new_entries = I_MAPALLOC(map, newcap * sizeof(ihstmap_entry_t));
	if (new_entries == NULL) {
		I_MAPFREE(map, new_ctrl);
		return IHSTMAP_ERRNOMEM;
	}

	IHM_MEMSET(new_ctrl, I_IHSTMAP_CTRL_EMPTY, newcap * sizeof(ihstmap_ctrl_t));

	old_ctrl    = (ihstmap_ctrl_t *)map->control;
	old_entries = (ihstmap_entry_t *)map->entries;
	oldcap      = map->capacity;

	for (i = 0; i < oldcap; ++i) {
		if (I_IHSTMAP_CTRL_IS_FULL(old_ctrl[i]))
			ihm__rehash_insert(new_ctrl, new_entries, newcap, map->h,
					   old_entries[i].key, old_entries[i].value);
	}

	I_MAPFREE(map, old_ctrl);
	I_MAPFREE(map, old_entries);

	map->control  = new_ctrl;
	map->entries  = new_entries;
	map->capacity = newcap;

	return IHSTMAP_OK;
}

int ihstmap_clear(ihstmap_t *map)
{
	if (map == NULL)
		return IHSTMAP_ERRINVAL;

	IHM_MEMSET(map->control, I_IHSTMAP_CTRL_EMPTY, map->capacity * sizeof(ihstmap_ctrl_t));

	map->size = 0;
	return IHSTMAP_OK;
}

/* UTILITY */

ilib_size_t ihstmap_size(const ihstmap_t *map)
{
	return map->size;
}

ilib_size_t ihstmap_capacity(const ihstmap_t *map)
{
	return map->capacity;
}

int ihstmap_empty(const ihstmap_t *map)
{
	return map->size == 0;
}

#endif /* I_IHSTMAP_IMPL_ */
#endif /* IHSTMAP_IMPLEMENTATION */
