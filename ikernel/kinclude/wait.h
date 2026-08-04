#ifndef WAIT_H_
#define WAIT_H_

#include <pthread.h>
#include <ilisti.h>

#define WNOHANG		0x00000001
#define WEXITED		0x00000004

typedef struct iwait_queue_entry iwait_queue_entry_t;

typedef int (*iwait_queue_func_t)(struct iwait_queue_entry *wq_entry, unsigned mode, int flags, void *key);
int default_wake_function(struct iwait_queue_entry *wq_entry, unsigned mode, int flags, void *key);

struct iwait_queue_entry {
	unsigned int		flags;
	void			*private;
	iwait_queue_func_t	func;
	struct ilinode		entry;
};

struct iwait_queue_head {
	pthread_mutex_t		lock;
	struct ilinode		head;
};

typedef struct iwait_queue_head wait_queue_head_t;

struct itask;

extern void add_wait_queue(struct iwait_queue_head *wq_head, struct iwait_queue_entry *wq_entry);
extern void remove_wait_queue(struct iwait_queue_head *wq_head, struct iwait_queue_entry *wq_entry);

static inline void init_waitqueue_func_entry(struct iwait_queue_entry *wq_entry, iwait_queue_func_t func)
{
	wq_entry->flags   = 0;
	wq_entry->private = NULL;
	wq_entry->func    = func;
}

static inline void init_waitqueue_entry(struct iwait_queue_entry *wq_entry, struct itask *p)
{
	wq_entry->flags   = 0;
	wq_entry->private = p;
	wq_entry->func    = default_wake_function;
}

static inline void __init_waitqueue_head(struct iwait_queue_head *wq_head)
{
	pthread_mutex_init(&wq_head->lock, NULL);
	ilisti_init(&wq_head->head);
}

#define init_waitqueue_head(wq_head)			\
	do {						\
		__init_waitqueue_head(wq_head);		\
	} while (0)

#endif // WAIT_H_
