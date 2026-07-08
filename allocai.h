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

#define AL_OK       0
#define AL_ERRNOMEM 1
#define AL_ERRBCKFL 2
#define AL_ERRINVAL 3

#define AL_ERRSTKOF 14
#define AL_ERRSTKUF 15

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

#endif /* ALLOCAI_H */

#define ALLOCAI_IMPLEMENTATION
#ifdef ALLOCAI_IMPLEMENTATION
#ifndef I_ALLOCAI_IMPLEMENTATION

int al_errno = AL_OK;

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
#endif /* I_ALLOC_USE_MMAP == 0 */

void *al_memset(void *ptr, int c, al_size_t n)
{
	al_byte_t *_ptr = (al_byte_t *)ptr;

	while (n--) {
		*_ptr++ = (al_byte_t)c;
	}

	return ptr;
}

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