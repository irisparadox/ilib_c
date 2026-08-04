#include <kinclude/ktid.h>
#include <kinclude/kfun.h>
#include <kinclude/sched.h>
#include <kinclude/syscall.h>

struct itask *tid_task(int tid)
{
	if (tid >= TID_MAX || tid < 0) return NULL;

	struct ktid_alloc *al = ktid_get_allocator();

	return al->slots[tid].thread;
}

struct itask *ktid_get_task(struct ktid *ktid)
{
	return ktid->thread;
}

struct ktid *tid_get_ktid(int tid)
{
	if (tid >= TID_MAX || tid < 0) return NULL;

	struct ktid_alloc *al = ktid_get_allocator();

	return &al->slots[tid];
}

struct ktid *get_task_ktid(struct itask *p)
{
	return p->tid;
}

tid_t task_tid_nr(struct itask *task)
{
	return task->tid->id;
}

tid_t ktid_nr(struct ktid *ktid)
{
	return ktid->id;
}

static struct ktid **task_ktid_ptr(struct itask *task)
{
	return &task->tid;
}

void attach_ktid(struct itask *task)
{
	struct ktid *ktid;

	ktid = *task_ktid_ptr(task);
	ktid->thread = task;
}

static void __change_ktid(struct itask *task, struct ktid *new)
{
	struct ktid **ktid_ptr, *ktid;

	ktid_ptr = task_ktid_ptr(task);
	ktid     = *ktid_ptr;

	if (ktid)
		ktid->thread = NULL;

	*ktid_ptr = new;

	if (new)
		new->thread = task;
}

void detach_ktid(struct itask *task)
{
	__change_ktid(task, NULL);
}

void change_ktid(struct itask *task, struct ktid *ktid)
{
	__change_ktid(task, ktid);
}

void ktid_init(struct ktid_alloc *al)
{
	tid_t i;

	pthread_mutex_init(&al->lock, NULL);
	ilisti_init(&al->ktid_head);
}

extern struct ktid_alloc *ktid_get_allocator(void)
{
	return &ikern_self()->tid_al;
}

struct ktid *ktid_alloc(void)
{
	struct ktid *ktid;
	struct ilinode *node;
	struct ktid_alloc *al = ktid_get_allocator();

	pthread_mutex_lock(&al->lock);

	node = ilisti_front(&al->ktid_head);
	if (node) {
		ktid = ILISTI_ENTRY(node, struct ktid, ktid_link);
		ilisti_remove(node);
	} else if (al->next_tid < TID_MAX) {
		ktid = &al->slots[al->next_tid];
		ktid->id = al->next_tid++;
	} else {
		pthread_mutex_unlock(&al->lock);
		return NULL;
	}

	pthread_mutex_unlock(&al->lock);
	return ktid;
}

void free_tid(struct ktid *ktid)
{
	struct ktid_alloc *al = ktid_get_allocator();
	
	pthread_mutex_lock(&al->lock);
	ilisti_push_front(&al->ktid_head, &ktid->ktid_link);
	pthread_mutex_unlock(&al->lock);
}

SYSCALL_DEFINE0(gettid)
{
	return current->tid->id;
}
