#ifndef WAIT_H_
#define WAIT_H_

#include <pthread.h>
#include <ilisti.h>

typedef struct iwait_queue_entry iwait_queue_entry_t;

typedef int (*iwait_queue_func_t)(struct iwait_queue_entry *wq_entry, unsigned mode, int flags, void *key);
int default_wake_function(struct iwait_queue_entry *wq_entry, unsigned mode, int flags, void *key);

struct iwait_queue_entry {
	unsigned int      	 flags;
	void              	*private;
	iwait_queue_func_t	 func;
	struct ilinode    	 entry;
};

struct iwait_queue_head {
	pthread_mutex_t	 lock;
	struct ilinode 	 head;
};

typedef struct iwait_queue_head wait_queue_head_t;

struct itask;

#endif // WAIT_H_
