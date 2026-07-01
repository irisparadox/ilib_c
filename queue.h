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

typedef unsigned int qsize_t;
typedef unsigned int esize_t;

typedef struct {
	void *data;
	qsize_t front, back, size, capacity;
	esize_t elem_size;
} queue_t;

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
#endif // QUEUE_H