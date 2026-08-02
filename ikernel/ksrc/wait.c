#include <kinclude/wait.h>

void add_wait_queue(struct iwait_queue_head *wq_head, struct iwait_queue_entry *wq_entry)
{
	pthread_mutex_lock(&wq_head->lock);
	ilisti_push_back(&wq_head->head, &wq_entry->entry);
	pthread_mutex_unlock(&wq_head->lock);
}

void remove_wait_queue(struct iwait_queue_head *wq_head, struct iwait_queue_entry *wq_entry)
{
	pthread_mutex_lock(&wq_head->lock);
	ilisti_remove(&wq_entry->entry);
	pthread_mutex_unlock(&wq_head->lock);
}
