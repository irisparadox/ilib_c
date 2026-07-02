/* SPDX-License-Identifier: MIT */

#ifndef QUEUE_H
#define QUEUE_H

#ifndef QUEUEF_STRERR
#define QUEUEF_STRERR 0
#endif /* QUEUEF_STRERR */

#ifndef QUEUEF_FLAG_ERR
#if QUEUEF_STRERR == 1
#define QUEUEF_FLAG_ERR 0
#else /* QUEUEF_STRERR */
#define QUEUEF_FLAG_ERR 1
#endif /* QUEUEF_STRERR */
#endif /* QUEUEF_FLAG_ERR */

#if QUEUEF_FLAG_ERR == 1
extern int que_err;
#endif /* QUEUEF_FLAG_ERR */

#define QUEINVAL    22
#define QUENOMEM    12
#define QUEAGAIN    11
#define QUEOVERFLOW 75
#define QUEFAULT    14

#ifndef QUE_DECL
#define QUE_DECL static inline
#endif /* QUE_DECL */

#ifndef PQ_DEL
#define PQ_DECL static inline
#endif /* PQ_DECL */

typedef unsigned int qsize_t;
typedef unsigned int esize_t;

typedef struct {
	void    *data;
	qsize_t  front, back, size, capacity;
	esize_t  elem_size;
} queue_t;

typedef int (*pq_cmp_t)(const void *a, const void *b);

typedef struct {
	void     *heap;
	qsize_t   size, capacity;
	esize_t   elem_size;
	pq_cmp_t  cmp;
} pqueue_t;

#if QUEUEF_STRERR == 1
typedef struct {
	int code;
	const char *op;
	qsize_t capacity;
	qsize_t size;
	void *ptr;
} queue_err_t;
#endif /* QUEUEF_STRERR */

#if QUEUEF_STRERR == 1
typedef queue_err_t que_ret_t;
#else /* QUEUEF_STRERR */
typedef int que_ret_t;
#endif /* QUEUEF_STRERR */

#ifndef QUE_RET
#define QUE_RET que_ret_t
#endif /* QUE_RET */

QUE_DECL QUE_RET queue_construct(queue_t *q, esize_t elem_size);
QUE_DECL QUE_RET queue_destroy(queue_t *q);

#define queue_push(q, type, v) i_queue_push((q), &(type){ (v) })
QUE_DECL QUE_RET queue_pop(queue_t *q);

#define queue_front(q, type) (*(type *)i_queue_front((q)))
QUE_DECL int queue_empty(const queue_t *q);

PQ_DECL QUE_RET pqueue_construct(pqueue_t *pq, qsize_t elem_size, pq_cmp_t cmp);
PQ_DECL QUE_RET pqueue_destroy(pqueue_t *pq);
PQ_DECL QUE_RET i_pqueue_push(pqueue_t *pq, const void *elem);
#define pqueue_push(pq, type, v) i_pqueue_push((pq), &(type){(v)})
PQ_DECL QUE_RET pqueue_pop(pqueue_t *pq);
#define pqueue_top(pq, type) (*(type *)i_pqueue_top((pq)))
PQ_DECL int pqueue_empty(const pqueue_t *pq);
#endif // QUEUE_H

//#define QUEUE_IMPLEMENTATION
#ifdef QUEUE_IMPLEMENTATION
#ifndef I_QUEUE_IMPLEMENTATION
#define I_QUEUE_IMPLEMENTATION

#if !((defined(QUE_MALLOC) == defined(QUE_FREE)) && (defined(QUE_FREE) == defined(QUE_REALLOC)))
#error "You must either use the default allocator or provide all of: QUE_MALLOC, QUE_REALLOC and QUE_FREE"
#endif

#if !defined(QUE_MALLOC) || !defined(QUE_FREE) || !defined(QUE_REALLOC)

#include <stdlib.h>

#ifndef QUE_MALLOC
#define QUE_MALLOC malloc
#endif /* QUE_MALLOC */

#ifndef QUE_FREE
#define QUE_FREE free
#endif /* QUE_FREE */

#ifndef QUE_REALLOC
#define QUE_REALLOC realloc
#endif /* QUE_REALLOC */

#endif /* !defined(QUE_MALLOC) || !defined(QUE_FREE) || !defined(QUE_REALLOC) */

#if !(defined(QUE_MEMSET) == defined(QUE_MEMCPY))
#error "You must either use the default mem ops or provide QUE_MEMSET and QUE_MEMCPY"
#endif

#if !defined(QUE_MEMSET) || !defined(QUE_MEMCPY)

#include <string.h>

#ifndef QUE_MEMSET
#define QUE_MEMSET memset
#endif /* QUE_MEMSET */

#ifndef QUE_MEMCPY
#define QUE_MEMCPY memcpy
#endif /* QUE_MEMCPY */

#endif /* !defined(QUE_MEMSET) || !defined(QUE_MEMCPY) */

QUE_DECL QUE_RET queue_construct(queue_t *q, esize_t elem_size)
{
#if QUEUEF_STRERR
	queue_err_t res = {0};
	res.op = "queue_construct";
#endif

	if (!q || elem_size == 0) {
#if QUEUEF_FLAG_ERR
		que_err = QUEINVAL;
#endif

#if QUEUEF_STRERR
		res.code = QUEINVAL;
		return (res);
#else
		return (QUEINVAL);
#endif
	}

	q->capacity = 8;
	q->front = 0;
	q->back = 0;
	q->size = 0;
	
	q->elem_size = elem_size;

	q->data = QUE_MALLOC(q->capacity * elem_size);

	if (!q->data) {
#if QUEUEF_FLAG_ERR
		que_err = QUENOMEM;
#endif

#if QUEUEF_STRERR
		res.code = QUENOMEM;
		res.capacity = q->capacity;
		return (res);
#else
		return (QUENOMEM);
#endif
	}

#if QUEUEF_STRERR
	res.capacity = q->capacity;
	res.size = 0;
	res.ptr = q->data;
	return (res);
#else
	return 0;
#endif
}

QUE_DECL QUE_RET queue_destroy(queue_t *q)
{
#if QUEUEF_STRERR
	queue_err_t res = {0};
	res.op = "queue_destroy";
#endif

	if (!q) {
#if QUEUEF_FLAG_ERR
		que_err = QUEINVAL;
#endif

#if QUEUEF_STRERR
		res.code = QUEINVAL;
		return (res);
#else
		return (QUEINVAL);
#endif
	}

	if (q->data) {
		QUE_FREE(q->data);
		q->data = NULL;
	}

	q->front = 0;
	q->back = 0;
	q->size = 0;
	q->capacity = 0;
	q->elem_size = 0;

#if QUEUEF_STRERR
	return (res);
#else
	return 0;
#endif
}

QUE_DECL QUE_RET i_queue_push(queue_t *q, const void *elem)
{
#if QUEUEF_STRERR
	queue_err_t res = {0};
	res.op = "queue_push";
#endif

	if (!q || !elem) {
#if QUEUEF_FLAG_ERR
		que_err = QUEINVAL;
#endif

#if QUEUEF_STRERR
		res.code = QUEINVAL;
		return (res);
#else
		return QUEINVAL;
#endif
	}

	if (q->size == q->capacity) {
		qsize_t new_cap = q->capacity << 1;
		void *new_data = QUE_MALLOC(new_cap * q->elem_size);

#if QUEUEF_STRERR
		if (!new_data)
		{
			res.code = QUENOMEM;
			return res;
		}
#else
		if (!new_data)
			return QUENOMEM;
#endif

		for (qsize_t i = 0; i < q->size; ++i) {
			qsize_t idx = (q->front + i) & (q->capacity - 1);
			QUE_MEMCPY((char *)new_data + i * q->elem_size,
				(char *)q->data + idx * q->elem_size,
				q->elem_size);
		}

		QUE_FREE(q->data);
		q->data = new_data;

		q->capacity = new_cap;
		q->front = 0;
		q->back = q->size;
	}

	QUE_MEMCPY((char *)q->data + q->back * q->elem_size, elem, q->elem_size);

	q->back = (q->back + 1) & (q->capacity - 1);
	q->size++;

#if QUEUEF_STRERR
	res.capacity = q->capacity;
	res.size = q->size;
	res.ptr = q->data;
	return (res);
#else
	return 0;
#endif
}

QUE_DECL QUE_RET queue_pop(queue_t *q)
{
#if QUEUEF_STRERR
	queue_err_t res = {0};
	res.op = "queue_pop";
#endif

	if (!q || !q->data || q->size == 0) {
#if QUEUEF_FLAG_ERR
		que_err = QUEINVAL;
#endif

#if QUEUEF_STRERR
		res.code = QUEINVAL;
		return (res);
#else
		return (QUEINVAL);
#endif
	}

	q->front = (q->front + 1) & (q->capacity - 1);
	q->size--;

#if QUEUEF_STRERR
	res.size = q->size;
	res.capacity = q->capacity;
	res.ptr = q->data;
	return res;
#else
	return 0;
#endif
}

QUE_DECL void *i_queue_front(const queue_t *q)
{
	if (!q || !q->data || q->size == 0) {
#if QUEUEF_FLAG_ERR
		que_err = QUEINVAL;
#endif
		return NULL;
	}

	return (char *)q->data + (q->front * q->elem_size);
}

QUE_DECL int queue_empty(const queue_t *q)
{
	return !q || !q->data || q->size == 0;
}

#endif // I_QUEUE_IMPLEMENTATION
#endif // QUEUE_IMPLEMENTATION

//#define PQUEUE_IMPLEMENTATION
#ifdef PQUEUE_IMPLEMENTATION
#ifndef I_PQUE_IMPLEMENTATION
#define I_PQUE_IMPLEMENTATION

#if !((defined(PQ_MALLOC) == defined(PQ_FREE)) && (defined(PQ_FREE) == defined(PQ_REALLOC)))
#error "You must either use the default allocator or provide all of: PQ_MALLOC, PQ_REALLOC and PQ_FREE"
#endif

#if !defined(PQ_MALLOC) || !defined(PQ_FREE) || !defined(PQ_REALLOC)

#include <stdlib.h>

#ifndef PQ_MALLOC
#define PQ_MALLOC malloc
#endif /* PQ_MALLOC */

#ifndef PQ_FREE
#define PQ_FREE free
#endif /* PQ_FREE */

#ifndef PQ_REALLOC
#define PQ_REALLOC realloc
#endif /* PQ_REALLOC */

#endif /* !defined(PQ_MALLOC) || !defined(PQ_FREE) || !defined(PQ_REALLOC) */

#if !(defined(PQ_MEMSET) == defined(PQ_MEMCPY))
#error "You must either use the default mem ops or provide PQ_MEMSET and PQ_MEMCPY"
#endif

#if !defined(PQ_MEMSET) || !defined(PQ_MEMCPY)

#include <string.h>

#ifndef PQ_MEMSET
#define PQ_MEMSET memset
#endif /* PQ_MEMSET */

#ifndef PQ_MEMCPY
#define PQ_MEMCPY memcpy
#endif /* PQ_MEMCPY */

#endif /* !defined(PQ_MEMSET) || !defined(PQ_MEMCPY) */

PQ_DECL QUE_RET pqueue_construct(pqueue_t *pq, qsize_t elem_size, pq_cmp_t cmp)
{
#if QUEUEF_STRERR
	queue_err_t res = {0};
	res.op = "pqueue_construct";
#endif

	if (!pq || elem_size == 0 || !cmp) {
#if QUEUEF_FLAG_ERR
		que_err = QUEINVAL;
#endif

#if QUEUEF_STRERR
		res.code = QUEINVAL;
		return (res);
#else
		return (QUEINVAL);
#endif
	}

	pq->capacity  = 8;
	pq->size      = 0;
	pq->elem_size = elem_size;
	pq->cmp	      = cmp;
	pq->heap      = PQ_MALLOC(pq->capacity * elem_size);

	if (!pq->heap) {
#if QUEUEF_FLAG_ERR
		que_err = QUENOMEM;
#endif

#if QUEUEF_STRERR
		res.code = QUENOMEM;
		res.capacity = pq->capacity;
		return (res);
#else
		return (QUENOMEM);
#endif
	}

#if QUEUEF_STRERR
	res.capacity = pq->capacity;
	res.size = 0;
	res.ptr = pq->heap;
	return (res);
#else
	return 0;
#endif
}

PQ_DECL QUE_RET pqueue_destroy(pqueue_t *pq)
{
#if QUEUEF_STRERR
	queue_err_t res = {0};
	res.op = "pqueue_destroy";
#endif
	if (!pq) {
#if QUEUEF_FLAG_ERR
		que_err = QUEINVAL;
#endif
#if QUEUEF_STRERR
		res.code = QUEINVAL;
		return (res);
#else
		return (QUEINVAL);
#endif
	}

	PQ_FREE(pq->heap);
	pq->heap      = NULL;
	pq->size      = 0;
	pq->capacity  = 0;
	pq->elem_size = 0;
	pq->cmp       = NULL;

#if QUEUEF_STRERR
	res.code     = 0;
	res.capacity = 0;
	res.size     = 0;
	res.ptr      = NULL;
	return (res);
#else
	return 0;
#endif
}

static void
i_pqueue_sift_up(pqueue_t *pq, qsize_t idx)
{
	char *tmp = (char *)PQ_MALLOC(pq->elem_size);

	while (idx > 0) {
		qsize_t parent = (idx - 1) / 2;
		void *child_ptr  = (char *)pq->heap + idx * pq->elem_size;
		void *parent_ptr = (char *)pq->heap + parent * pq->elem_size;

		if (pq->cmp(child_ptr, parent_ptr) >= 0) break;

		PQ_MEMCPY(tmp, child_ptr, pq->elem_size);
		PQ_MEMCPY(child_ptr, parent_ptr, pq->elem_size);
		PQ_MEMCPY(parent_ptr, tmp, pq->elem_size);

		idx = parent;
	}

	PQ_FREE(tmp);
}

PQ_DECL QUE_RET i_pqueue_push(pqueue_t *pq, const void *elem)
{
#if QUEUEF_STRERR
	queue_err_t res = {0};
	res.op = "pqueue_push";
#endif
	if (!pq || !elem) {
#if QUEUEF_FLAG_ERR
		que_err = QUEINVAL;
#endif
#if QUEUEF_STRERR
		res.code = QUEINVAL;
		return (res);
#else
		return (QUEINVAL);
#endif
	}

	if (pq->size == pq->capacity) {
		qsize_t new_cap = pq->capacity << 1;
		void *new_heap = PQ_MALLOC(new_cap * pq->elem_size);
#if QUEUEF_STRERR
		if (!new_heap) {
			res.code = QUENOMEM;
			return (res);
		}
#else
		if (!new_heap) return (QUENOMEM);
#endif
		PQ_MEMCPY(new_heap, pq->heap, pq->size * pq->elem_size);
		PQ_FREE(pq->heap);
		pq->heap = new_heap;
		pq->capacity = new_cap;
	}

	PQ_MEMCPY((char *)pq->heap + pq->size * pq->elem_size, elem, pq->elem_size);
	i_pqueue_sift_up(pq, pq->size);
	pq->size++;

#if QUEUEF_STRERR
	res.capacity = pq->capacity;
	res.size = pq->size;
	res.ptr = pq->heap;
	return (res);
#else
	return 0;
#endif
}

#endif // I_PQUE_IMPLEMENTATION
#endif // PQUEUE_IMPLEMENTATION

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