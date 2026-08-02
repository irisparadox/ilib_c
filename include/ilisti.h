#ifndef ILISTI_H_
#define ILISTI_H_

#include <deftypei.h>

typedef struct ilinode {
	struct ilinode *next;
	struct ilinode *prev;
} ilinode_t;

extern void ilisti_init(ilinode_t *head);
extern int  ilisti_empty(const ilinode_t *head);

extern void ilisti_insert_before(ilinode_t *pos, ilinode_t *node);
extern void ilisti_insert_after(ilinode_t *pos, ilinode_t *node);
extern void ilisti_push_front(ilinode_t *head, ilinode_t *node);
extern void ilisti_push_back(ilinode_t *head, ilinode_t *node);

extern void ilisti_remove(ilinode_t *node);

extern ilinode_t *ilisti_front(const ilinode_t *head);
extern ilinode_t *ilisti_back(const ilinode_t *head);

extern ilinode_t *ilisti_pop_front(ilinode_t *head);
extern ilinode_t *ilisti_pop_back(ilinode_t *head);

#define ILISTI_ENTRY(ptr, type, member) \
	((type *)((char *)(ptr) - ILIB_OFFSETOF(type, member)))

#define ILISTI_FOREACH(pos, head) \
	for ((pos) = (head)->next; (pos) != (head); (pos) = (pos)->next)
#define ILISTI_FOREACH_SAFE(pos, tmp, head)              \
	for ((pos) = (head)->next, (tmp) = (pos)->next;  \
	     (pos) != (head);                            \
	     (pos) = (tmp), (tmp) = (pos)->next)

#define list_first_entry(head, type, member) \
	ILISTI_ENTRY((head)->next, type, member)

#define list_next_entry(pos, member) \
	ILISTI_ENTRY((pos)->member.next, __typeof__(*(pos)), member)

#define list_for_each_entry(pos, head, member)                          	\
	for ((pos) = list_first_entry(head, __typeof__(*(pos)), member);	\
		&(pos)->member != (head);                               	\
		(pos) = list_next_entry(pos, member))

#endif // ILISTI_H_
