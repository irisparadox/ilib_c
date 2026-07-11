#ifndef ALLOCAI_H
#define ALLOCAI_H

#ifndef ALLOC_F_FALLBACK
#define ALLOC_F_FALLBACK 0
#endif /* ALLOC_FALLBACK */

#if ALLOC_F_FALLBACK == 0
#if !(defined(__linux__) && defined(__GNUC__) && (defined(__x86_64__) || defined(__aarch64__)))
#error "The current system is not supported for default behaviour, please enable ALLOC_F_FALLBACK"
#else
#define I_ALLOC_USE_MMAP 1
#endif /* !(defined(__linux__) && defined(__GNUC__) && (defined(__x86_64__) || defined(_M_X64))) */
#else
#define I_ALLOC_USE_MMAP 0
#endif /* ALLOC_F_FALLBACK == 0 */

#define AL_OK        0 /* OK */
#define AL_ERRNOMEM  1 /* NO MEMORY AVAILABLE */
#define AL_ERRBCKFL  2 /* BACKEND FAILED */
#define AL_ERRINVAL  3 /* INVALID ARG */

#define AL_ERRSTKOF 14 /* STACK OVERFLOW */
#define AL_ERRSTKUF 15 /* STACK UNDERFLOW */

#define AL_ERRINPTR 64 /* HEAP INVALID PTR */
#define AL_ERRMGCHK 65 /* HEAP MAGIC CHECK FAIL */
#define AL_ERRAGAIN 66 /* DOUBLE FREE */

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef AL_STACK_MAX_MARKS
#define AL_STACK_MAX_MARKS 32
#endif /* AL_STACK_MAX_MARKS */

extern int al_errno;

typedef unsigned char al_byte_t;
typedef unsigned long al_size_t;

typedef union {
	void *p;
	long long ll;
	double d;
	void (*fp)(void);
} al_max_align_t;

#define ALLOC_ALIGNMENT (sizeof(al_max_align_t))
#define AL_SIZE_MAX     ((al_size_t)-1)

typedef struct {
	al_byte_t *base;
	al_size_t  offset;
	al_size_t  capacity;
} al_arena_t;

typedef struct {
	al_arena_t arena;
	al_size_t  marks[AL_STACK_MAX_MARKS];
	al_size_t  top;
} al_stack_t;

void *al_memset(void *ptr, int c, al_size_t n);
void *al_memcpy(void *restrict dest, const void *restrict src, al_size_t n);

al_arena_t  al_arena_init(al_size_t capacity);
al_byte_t  *al_arena_alloc(al_arena_t *a, al_size_t size);
al_byte_t  *al_arena_calloc(al_arena_t *a, al_size_t nmemb, al_size_t size);
void        al_arena_reset(al_arena_t *a);
void        al_arena_free(al_arena_t *a);

al_stack_t  al_stack_init(al_size_t capacity);
al_byte_t  *al_stack_alloc(al_stack_t *s, al_size_t size);
void        al_stack_push(al_stack_t *s);
void        al_stack_pop(al_stack_t *s);
void        al_stack_free(al_stack_t *s);

#ifndef ALLOC_F_HEAP
#define ALLOC_F_HEAP 0
#endif /* ALLOCAI_F_HEAP */

#if ALLOC_F_HEAP == 1

#ifndef ALLOCAI_REGION_GROWTH
#define ALLOCAI_REGION_GROWTH (4 * 1024 * 1024)
#endif /* ALLOCAI_REGION_GROWTH */

#ifndef AL_MIN_BLOCK_SIZE 
#define AL_MIN_BLOCK_SIZE 2 * ALLOC_ALIGNMENT
#endif /* AL_STACK_MAX_MARKS */

#include "rbtree.h"

typedef struct ha_region ha_region_t;
typedef struct ha_block  ha_block_t;

struct ha_region {
	rb_node_t    rb;

	void        *base;
	al_size_t    size;

	ha_region_t *next;
	ha_block_t  *first;
};

struct ha_block {
	al_size_t   size;
	unsigned    magic;
	unsigned    free;
	ha_block_t *next;
	ha_block_t *prev;
};

typedef struct {
	ha_region_t *regions;
	rb_tree_t    rbt;
} ha_allocator_t;

ha_allocator_t halloc_init(al_size_t size);
void           halloc_destroy(ha_allocator_t *h);

void *halloc(ha_allocator_t *h, al_size_t size);
void *hcalloc(ha_allocator_t *h, al_size_t nmemb, al_size_t size);
void *hrealloc(ha_allocator_t *h, void *ptr, al_size_t size);
void  hfree(ha_allocator_t *h, void *ptr);

#endif /* ALLOC_F_HEAP == 1 */

#endif /* ALLOCAI_H */

#if defined(ALLOCAI_IMPLEMENTATION) || defined(ALLOCAI_HEAP_IMPLEMENTATION)
#ifndef I_ALLOCAI_ERRNO_GUARD
#define I_ALLOCAI_ERRNO_GUARD
int al_errno = AL_OK;
#endif /* I_ALLOCAI_ERRNO_GUARD */

#ifndef I_ALLOCAI_MEMOP_GUARD
#define I_ALLOCAI_MEMOP_GUARD
void *al_memset(void *ptr, int c, al_size_t n)
{
	al_byte_t *_ptr = (al_byte_t *)ptr;

	while (n--) {
		*_ptr++ = (al_byte_t)c;
	}

	return ptr;
}

void *al_memcpy(void *restrict dest, const void *restrict src, al_size_t n)
{
	unsigned char *d = dest;
	const unsigned char *s = src;

	while (n--) {
		*d++ = *s++;
	}

	return dest;
}
#endif /* I_ALLOCAI_MEMOP_GUARD */
#endif

//#define ALLOCAI_IMPLEMENTATION
#ifdef ALLOCAI_IMPLEMENTATION
#ifndef I_ALLOCAI_IMPLEMENTATION

#if (I_ALLOC_USE_MMAP == 0 || ALLOC_F_FALLBACK == 1)

#if !(defined(ALLOC_MALLOC) == defined(ALLOC_FREE))
#error "You must either use the default allocator or provide both of: ALLOC_MALLOC and ALLOC_FREE"
#endif /* !(defined(ALLOC_MALLOC) == defined(ALLOC_FREE)) */

#if !defined(ALLOC_MALLOC) || !defined(ALLOC_FREE)
#include <stdlib.h>
#ifndef ALLOC_MALLOC
#define ALLOC_MALLOC malloc
#endif /* ALLOC_MALLOC */
#ifndef ALLOC_FREE
#define ALLOC_FREE free
#endif /* ALLOC_FREE */
#endif /* !defined(ALLOC_MALLOC) || !defined(ALLOC_FREE) */

#else
#include <sys/mman.h>
#endif /* I_ALLOC_USE_MMAP == 0 || ALLOC_F_FALLBACK == 1 */

al_arena_t al_arena_init(al_size_t capacity)
{
	al_arena_t a;
	void *ret;

	if (capacity == 0) {
		al_errno   = AL_ERRINVAL;
		a.base     = NULL;
		a.offset   = 0;
		a.capacity = 0;
		return a;
	}

#if I_ALLOC_USE_MMAP
	ret = mmap(NULL,
		   capacity,
		   PROT_READ | PROT_WRITE,
		   MAP_PRIVATE | MAP_ANONYMOUS,
		   -1,
		   0);
	
	if (ret == MAP_FAILED) {
		al_errno   = AL_ERRBCKFL;
		a.base     = NULL;
		a.offset   = 0;
		a.capacity = 0;
		return a;
	}

	a.base = (al_byte_t *)ret;
#else
	a.base = (al_byte_t *)ALLOC_MALLOC(capacity);
	if (a.base == NULL) {
		al_errno   = AL_ERRBCKFL;
		a.offset   = 0;
		a.capacity = 0;
		return a;
	}
#endif

	al_errno   = AL_OK;
	a.offset   = 0;
	a.capacity = capacity;
	return a;
}

al_byte_t *al_arena_alloc(al_arena_t *a, al_size_t size)
{
	al_size_t  offset;
	al_byte_t *ptr;

	if (a == NULL || size == 0) {
		al_errno = AL_ERRINVAL;
		return NULL;
	}

	offset = (a->offset + (ALLOC_ALIGNMENT - 1)) & ~(ALLOC_ALIGNMENT - 1);

	if (offset > a->capacity || size > a->capacity - offset) {
		al_errno = AL_ERRNOMEM;
		return NULL;
	}

	ptr = a->base + offset;
	a->offset = offset + size;

	al_errno = AL_OK;
	return ptr;
}

al_byte_t *al_arena_calloc(al_arena_t *a, al_size_t nmemb, al_size_t size)
{
	al_size_t  total;
	al_byte_t *ptr;

	if (nmemb != 0 && size > AL_SIZE_MAX / nmemb) {
		al_errno = AL_ERRNOMEM;
		return NULL;
	}

	total = nmemb * size;

	ptr = al_arena_alloc(a, total);
	if (!ptr) return NULL;

	al_memset(ptr, 0, total);

	al_errno = AL_OK;
	return ptr;
}

void al_arena_reset(al_arena_t *a)
{
	if (!a) {
		al_errno = AL_ERRINVAL;
		return;
	}

	a->offset = 0;
	al_errno = AL_OK;
}

void al_arena_free(al_arena_t *a)
{
	if (!a) {
		al_errno = AL_ERRINVAL;
		return;
	}

	if (a->base == NULL) {
		al_errno = AL_OK;
		return;
	}

#if I_ALLOC_USE_MMAP
	if (munmap(a->base, a->capacity) != 0) {
		al_errno = AL_ERRBCKFL;
		return;
	}
#else
	ALLOC_FREE(a->base);
#endif
	a->base     = NULL;
	a->offset   = 0;
	a->capacity = 0;

	al_errno = AL_OK;
}

al_stack_t al_stack_init(al_size_t capacity)
{
	al_stack_t s;
	s.arena = al_arena_init(capacity);
	s.top   = 0;
	return s;
}

al_byte_t *al_stack_alloc(al_stack_t *s, al_size_t size)
{
	if (s == NULL) {
		al_errno = AL_ERRINVAL;
		return NULL;
	}

	return al_arena_alloc(&s->arena, size);
}

void al_stack_push(al_stack_t *s)
{
	if (s == NULL) {
		al_errno = AL_ERRINVAL;
		return;
	}

	if (s->top >= AL_STACK_MAX_MARKS) {
		al_errno = AL_ERRSTKOF;
		return;
	}

	s->marks[s->top] = s->arena.offset;
	++s->top;
	al_errno = AL_OK;
}

void al_stack_pop(al_stack_t *s)
{
	if (s == NULL) {
		al_errno = AL_ERRINVAL;
		return;
	}

	if (s->top <= 0) {
		al_errno = AL_ERRSTKUF;
		return;
	}

	--s->top;
	s->arena.offset = s->marks[s->top];
	
	al_errno = AL_OK;
}

void al_stack_free(al_stack_t *s)
{
	if (s == NULL) {
		al_errno = AL_ERRINVAL;
		return;
	}

	al_arena_free(&s->arena);
	s->top = 0;
	al_errno = AL_OK;
}

#endif /* I_ALLOCAI_IMPLEMENTATION */
#endif /* ALLOCAI_IMPLEMENTATION */

//#define ALLOCAI_HEAP_IMPLEMENTATION
#ifdef ALLOCAI_HEAP_IMPLEMENTATION
#ifndef I_ALLOCAI_HEAP_IMPL
#define I_ALLOCAI_HEAP_IMPL

#if !(defined(ALLOCAI_MMAP) == defined(ALLOCAI_MUNMAP))
#error "You must either not change the default memory manager or provide a valid API for ALLOCAI_MMAP and ALLOCAI_MUNMAP"
#endif /* !(defined(ALLOCAI_MMAP) == defined(ALLOCAI_MUNMAP)) */

#if !defined(ALLOCAI_MMAP) || !defined(ALLOCAI_MUNMAP)

#include <sys/mman.h>

#ifndef ALLOCAI_MMAP
#define ALLOCAI_MMAP(len) mmap(NULL, (len), PROT_READ | PROT_WRITE, \
				MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)
#endif /* ALLOCAI_MMAP */

#ifndef ALLOCAI_MUNMAP
#define ALLOCAI_MUNMAP(ptr, len) munmap((ptr), (len))
#endif /* ALLOCAI_MUNMAP */

#endif /* !defined(ALLOCAI_MMAP) || !defined(ALLOCAI_MUNMAP) */

#include "deftypei.h"
#define RBTREE_IMPLEMENTATION
#include "rbtree.h"

#define BLOCK_MAGIC 0x4D4A4D4A /* MJMJ */

/* compile-time check: header sizes must be multiples of ALLOC_ALIGNMENT,
 * or every user pointer handed out will silently be misaligned */
typedef char i_halloc_check_region_align[
	(sizeof(ha_region_t) % ALLOC_ALIGNMENT == 0) ? 1 : -1
];
typedef char i_halloc_check_block_align[
	(sizeof(ha_block_t) % ALLOC_ALIGNMENT == 0) ? 1 : -1
];

int region_cmp(const rb_node_t *a, const rb_node_t *b)
{
	const ha_region_t *ra = RB_CONTAINER(a, ha_region_t, rb);
	const ha_region_t *rb = RB_CONTAINER(b, ha_region_t, rb);

	return (ra->base > rb->base) - (ra->base < rb->base);
}

int region_key_cmp(const void *key, const rb_node_t *node)
{
	ilib_uintptr_t addr = (ilib_uintptr_t)key;
	const ha_region_t *r = RB_CONTAINER(node, ha_region_t, rb);

	ilib_uintptr_t begin = (ilib_uintptr_t)r->base;
	ilib_uintptr_t end   = begin + r->size;

	return (addr >= end) - (addr < begin);
}

static al_size_t i_halloc_align_up(al_size_t n)
{
	return (n + (ALLOC_ALIGNMENT - 1)) & ~(ALLOC_ALIGNMENT - 1);
}

static void *i_halloc_take(ha_block_t *b, al_size_t size)
{
	al_size_t remainder = b->size - size;

	if (remainder >= sizeof(ha_block_t) + AL_MIN_BLOCK_SIZE) {
		ha_block_t *split = (ha_block_t *)((al_byte_t *)(b + 1) + size);
		split->size  = remainder - sizeof(ha_block_t);
		split->magic = BLOCK_MAGIC;
		split->free  = 1;
		split->next  = b->next;
		split->prev  = b;
		if (b->next != NULL) {
			b->next->prev = split;
		}
		b->next = split;
		b->size = size;
	}

	b->free  = 0;
	al_errno = AL_OK;
	return (void *)(b + 1);
}

static void *i_halloc_try(ha_allocator_t *h, al_size_t size)
{
	ha_region_t *r;
	ha_block_t  *b;

	for (r = h->regions; r != NULL; r = r->next) {
		for (b = r->first; b != NULL; b = b->next) {
			if (b->free && b->size >= size) {
				return i_halloc_take(b, size);
			}
		}
	}

	return NULL;
}

static int i_halloc_grow(ha_allocator_t *h, al_size_t size)
{
	ha_region_t *new_region;
	ha_block_t  *new_block;
	al_size_t    region_size; 
	al_size_t    grow_size;
	void        *mem;

	if (size > AL_SIZE_MAX - sizeof(ha_region_t) - sizeof(ha_block_t)) {
		al_errno = AL_ERRNOMEM;
		return 0;
	}

	region_size = size + sizeof(ha_region_t) + sizeof(ha_block_t);
	grow_size   = region_size > ALLOCAI_REGION_GROWTH ?
			region_size : ALLOCAI_REGION_GROWTH;

	mem = ALLOCAI_MMAP(grow_size);
	if (mem == NULL) {
		al_errno = AL_ERRBCKFL;
		return 0;
	}

	new_region = (ha_region_t *)mem;
	new_region->base = mem;
	new_region->size = grow_size;
	new_region->next = h->regions;

	new_block = (ha_block_t *)((al_byte_t *)mem + sizeof(ha_region_t));
	new_block->size = grow_size - sizeof(ha_region_t) - sizeof(ha_block_t);
	new_block->magic = BLOCK_MAGIC;
	new_block->free  = 1;
	new_block->next  = NULL;
	new_block->prev  = NULL;

	new_region->first = new_block;
	h->regions = new_region;

	rb_insert(&h->rbt, &new_region->rb);

	return 1;
}

static int i_halloc_validate_ptr(ha_allocator_t *h, void *ptr)
{
	rb_node_t *reg = rb_search(&h->rbt, ptr);
	
	if (reg == NULL) {
		al_errno = AL_ERRINPTR;
		return 0;
	}

	return 1;
}

ha_allocator_t halloc_init(al_size_t size)
{
	ha_allocator_t  h;
	ha_region_t    *r;
	ha_block_t     *b;

	if (size == 0 || size <= sizeof(ha_region_t) + sizeof(ha_block_t)) {
		al_errno  = AL_ERRINVAL;
		h.regions = NULL;
		return h;
	}

	// metadata lives at the start of mmap block
	void *mem = ALLOCAI_MMAP(size);
	if (mem == NULL) {
		al_errno  = AL_ERRBCKFL;
		h.regions = NULL;
		return h;
	}

	r = (ha_region_t *)mem;
	r->base = mem;
	r->size = size;
	r->next = NULL;

	// the single free block covers everything after the region header
	b = (ha_block_t *)((al_byte_t *)mem + sizeof(ha_region_t));
	b->size = size - sizeof(ha_region_t) - sizeof(ha_block_t);
	b->magic = BLOCK_MAGIC;
	b->free = 1;
	b->next = NULL;
	b->prev = NULL;

	r->first  = b;
	h.regions = r;

	rb_init(&h.rbt, region_cmp, region_key_cmp);
	rb_insert(&h.rbt, &r->rb);

	al_errno = AL_OK;
	return h;
}

void halloc_destroy(ha_allocator_t *h)
{
	ha_region_t *r;
	ha_region_t *next;

	if (h == NULL) {
		al_errno = AL_ERRINVAL;
		return;
	}

	r = h->regions;
	while (r != NULL) {
		next = r->next;
		ALLOCAI_MUNMAP(r->base, r->size);
		r = next;
	}

	h->regions = NULL;
	al_errno = AL_OK;
}

void *halloc(ha_allocator_t *h, al_size_t size)
{
	void *found;

	if (h == NULL || size == 0) {
		al_errno = AL_ERRINVAL;
		return NULL;
	}

	size = i_halloc_align_up(size);

	found = i_halloc_try(h, size);
	if (found != NULL) {
		return found;
	}

	if (!i_halloc_grow(h, size)) {
		return NULL;
	}

	return i_halloc_take(h->regions->first, size);
}

void *hcalloc(ha_allocator_t *h, al_size_t nmemb, al_size_t size)
{
	al_size_t  total;
	void      *ptr;

	if (nmemb != 0 && size > AL_SIZE_MAX / nmemb) {
		al_errno = AL_ERRNOMEM;
		return NULL;
	}

	total = nmemb * size;
	ptr = halloc(h, total);
	if (ptr == NULL) {
		return NULL;
	}

	al_memset(ptr, 0, total);
	al_errno = AL_OK;
	return ptr;
}

void *hrealloc(ha_allocator_t *h, void *ptr, al_size_t size)
{
	ha_block_t *b;
	void       *new_ptr;
	al_size_t   copy_size;

	if (h == NULL) {
		al_errno = AL_ERRINVAL;
		return NULL;
	}

	if (ptr == NULL) {
		return halloc(h, size);
	}

	if (size == 0) {
		hfree(h, ptr);
		return NULL;
	}

	if (!i_halloc_validate_ptr(h, ptr)) return NULL;

	size = i_halloc_align_up(size);

	b = (ha_block_t *)ptr - 1;

	if (b->size >= size) {
		al_errno = AL_OK;
		return ptr;
	}

	if (b->next != NULL && b->next->free &&
	    b->size + sizeof(ha_block_t) + b->next->size >= size) {
		ha_block_t *r = b->next;

		b->size += sizeof(ha_block_t) + r->size;
		b->next  = r->next;
		if (r->next != NULL) {
			r->next->prev = b;
		}

		return i_halloc_take(b, size);
	}

	new_ptr = halloc(h, size);
	if (new_ptr == NULL) {
		return NULL;
	}

	copy_size = b->size;
	al_memcpy(new_ptr, ptr, copy_size);
	hfree(h, ptr);

	al_errno = AL_OK;
	return new_ptr;
}

void hfree(ha_allocator_t *h, void *ptr)
{
	ha_block_t *b;

	if (h == NULL) {
		al_errno = AL_ERRINVAL;
		return;
	}

	if (ptr == NULL) {
		al_errno = AL_OK;
		return;
	}

	if (!i_halloc_validate_ptr(h, ptr)) return;

	b = (ha_block_t *)ptr - 1;

	if (b->magic != BLOCK_MAGIC) {
		al_errno = AL_ERRMGCHK;
		return;
	}

	if (b->free) {
		al_errno = AL_ERRAGAIN;
		return;
	}

	b->free = 1;

	ha_block_t *r = b->next;
	ha_block_t *l = b->prev;
	if (r != NULL && r->free) {
		b->size += r->size + sizeof(ha_block_t);
		b->next  = r->next;
		if (r->next != NULL) {
			r->next->prev = b;
		}
		r->size = 0;
		r->next = NULL;
		r->prev = NULL;
	}

	if (l != NULL && l->free) {
		l->size += b->size + sizeof(ha_block_t);
		l->next = b->next;
		if (b->next != NULL) {
			b->next->prev = l;
		}
		b->size = 0;
		b->next = NULL;
		b->prev = NULL;
	}

	al_errno = AL_OK;
}

#endif /* ALLOCAI_HEAP_IMPLEMENTATION */
#endif /* I_ALLOCAI_HEAP_IMPL */

/*
 * MIT License
 *
 * Copyright (c) 2026 Gabriel Guillermo Saavedra Moreira
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
