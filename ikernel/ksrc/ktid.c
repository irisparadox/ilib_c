#include <kinclude/ktid.h>
#include <kinclude/kfun.h>

tid_t tid_alloc(struct ktid_alloc *al)
{
	tid_t tid;

	pthread_mutex_lock(&al->lock);
	for (tid = 0; tid < TID_MAX; ++tid) {
		if (bitmap_test(al->bitmap, tid)) {
			pthread_mutex_unlock(&al->lock);
			return tid;
		}
	}

	pthread_mutex_unlock(&al->lock);
	return TID_MAX;
}

void free_tid(struct ktid_alloc *al, tid_t tid)
{
	if (tid >= TID_MAX || tid < 0)
		BUG();

	pthread_mutex_lock(&al->lock);
	bitmap_clear(al->bitmap, tid);
	pthread_mutex_unlock(&al->lock);
}
