#include <ilisti.h>

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

ilinode_t *ilisti_pop_front(ilinode_t *head)
{
	struct ilinode *n;

	if (head == NULL) return NULL;

	n = ilisti_front(head);
	ilisti_remove(n);

	return n;
}

ilinode_t *ilisti_pop_back(ilinode_t *head)
{
	struct ilinode *n;

	if (head == NULL) return NULL;

	n = ilisti_back(head);
	ilisti_remove(n);

	return n;
}
