#ifndef ILISTI_H_
#define ILISTI_H_

#include <deftypei.h>

typedef struct ilinode {
	struct ilinode *next;
	struct ilinode *prev;
} ilinode_t;

void ilisti_init(ilinode_t *head);
int  ilisti_empty(const ilinode_t *head);

void ilisti_insert_before(ilinode_t *pos, ilinode_t *node);
void ilisti_insert_after(ilinode_t *pos, ilinode_t *node);
void ilisti_push_front(ilinode_t *head, ilinode_t *node);
void ilisti_push_back(ilinode_t *head, ilinode_t *node);

void ilisti_remove(ilinode_t *node);

ilinode_t *ilisti_front(const ilinode_t *head);
ilinode_t *ilisti_back(const ilinode_t *head);

#define ILISTI_ENTRY(ptr, type, member) \
	((type *)((char *)(ptr) - ILIB_OFFSETOF(type, member)))

#define ILISTI_FOREACH(pos, head) \
	for ((pos) = (head)->next; (pos) != (head); (pos) = (pos)->next)
#define ILISTI_FOREACH_SAFE(pos, tmp, head)              \
	for ((pos) = (head)->next, (tmp) = (pos)->next;  \
	     (pos) != (head);                            \
	     (pos) = (tmp), (tmp) = (pos)->next)

#endif // ILISTI_H_

//#define ILISTI_IMPLEMENTATION
#ifdef ILISTI_IMPLEMENTATION
#ifndef I_LSTI_IMPL
#define I_LSTI_IMPL

/* private  */
static inline void i_lsti_insert(ilinode_t *node, ilinode_t *prev, ilinode_t *next)
{
	prev->next = node;
	node->prev = prev;

	node->next = next;
	next->prev = node;
}

static inline void i_lsti_remove(ilinode_t *prev, ilinode_t *next)
{
	prev->next = next;
	next->prev = prev;
}

void ilisti_init(ilinode_t *head)
{
	if (head == NULL) return;
	head->next = head;
	head->prev = head;
}

int ilisti_empty(const ilinode_t *head)
{
	return head->next == head;
}

void ilisti_insert_before(ilinode_t *pos, ilinode_t *node)
{
	if (pos == NULL || node == NULL) return;

	i_lsti_insert(node, pos->prev, pos);
}

void ilisti_insert_after(ilinode_t *pos, ilinode_t *node)
{
	if (pos == NULL || node == NULL) return;

	i_lsti_insert(node, pos, pos->next);
}

void ilisti_push_front(ilinode_t *head, ilinode_t *node)
{
	if (head == NULL || node == NULL) return;

	i_lsti_insert(node, head, head->next);
}

void ilisti_push_back(ilinode_t *head, ilinode_t *node)
{
	if (head == NULL || node == NULL) return;
	i_lsti_insert(node, head->prev, head);
}

void ilisti_remove(ilinode_t *node)
{
	if (node == NULL || (node->next == node && node->prev == node)) return;

	i_lsti_remove(node->prev, node->next);

	node->next = node;
	node->prev = node;
}

ilinode_t *ilisti_front(const ilinode_t *head)
{
	if (head == NULL) return NULL;
	return head->next;
}

ilinode_t *ilisti_back(const ilinode_t *head)
{
	if (head == NULL) return NULL;
	return head->prev;
}

#endif /* I_LSTI_IMPL */
#endif /* ILISTI_IMPLEMENTATION */
