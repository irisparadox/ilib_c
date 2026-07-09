/*
 * rbtree.h -- single-header intrusive red-black tree
 *
 * SPDX-License-Identifier: MIT
 *
 * Intrusive balanced binary search tree implementing the
 * classic red-black tree described by Bayer, Guibas, and
 * Sedgewick. Maintains O(log n) search, insertion, and
 * deletion through recoloring and rotations.
 *
 * Nodes are intrusive: each object embeds an rb_node_t,
 * and the tree never allocates or frees user objects.
 * Recover the containing object with RB_CONTAINER().
 *
 * Usage:
 *     #define RBTREE_IMPLEMENTATION
 *     #include "rbtree.h"
 *
 * CONFIGURATION
 *     RBTREE_DECL                         Linkage/inline specifier
 *                                        for declarations.
 *                                        (default: static inline)
 *
 * CONTAINER MODEL
 *     rb_node_t       Intrusive node embedded within a user type.
 *     rb_tree_t       Tree descriptor holding the root and user-
 *                     supplied comparison callbacks.
 *     RB_CONTAINER()  Recover the enclosing structure from an
 *                     embedded rb_node_t.
 *
 * COMPARISON CALLBACKS
 *     rb_cmp_t        Compare two tree nodes. Used by insertion.
 *     rb_key_cmp_t    Compare an external search key against a
 *                     tree node. Used by rb_search().
 *
 * INITIALIZATION
 *     rb_init         Initialize a tree with node and key
 *                     comparison callbacks.
 *
 * MODIFICATION
 *     rb_insert       Insert a node while maintaining the
 *                     red-black invariants.
 *     rb_delete       Remove a node while maintaining the
 *                     red-black invariants.
 *
 * LOOKUP
 *     rb_search       Search for a node matching an external key.
 *     rb_minimum      Return the leftmost node in a subtree.
 *     rb_maximum      Return the rightmost node in a subtree.
 *     rb_successor    Return the inorder successor of a node,
 *                     or NULL if none exists.
 *     rb_predecessor  Return the inorder predecessor of a node,
 *                     or NULL if none exists.
 *
 * MEMORY OWNERSHIP
 *     The tree owns no memory. Nodes must remain valid while
 *     linked into a tree and are never allocated or freed by
 *     this library. Removing a node only unlinks it; lifetime
 *     management is entirely the caller's responsibility.
 */
 
#ifndef RBTREE_H
#define RBTREE_H

#ifndef RBTREE_DECL
#define RBTREE_DECL static inline
#endif /* RBTREE_DECL */

#ifndef NULL
#define NULL ((void *)0)
#endif /* NULL */

#include "deftypei.h"

#define RB_CONTAINER(ptr, type, member) \
	((type *)((char *)(ptr) - ILIB_OFFSETOF(type, member)))

typedef enum {
	RB_RED,
	RB_BLACK
} rb_color_t;

typedef struct rb_node rb_node_t;

struct rb_node {
	rb_node_t *parent;
	rb_node_t *left;
	rb_node_t *right;

	rb_color_t color;
};

typedef int (*rb_cmp_t)(const rb_node_t *key, const rb_node_t *r);
typedef int (*rb_key_cmp_t)(const void *key, const rb_node_t *r);

typedef struct {
	rb_node_t    *root;
	rb_cmp_t      cmp;
	rb_key_cmp_t  kcmp;
} rb_tree_t;

RBTREE_DECL void rb_init(rb_tree_t *t, rb_cmp_t cmp, rb_key_cmp_t kcmp);
RBTREE_DECL void rb_insert(rb_tree_t *t, rb_node_t *node);
RBTREE_DECL void rb_delete(rb_tree_t *t, rb_node_t *node);

RBTREE_DECL rb_node_t *rb_search(const rb_tree_t *t, const void *key);
RBTREE_DECL rb_node_t *rb_minimum(rb_node_t *t);
RBTREE_DECL rb_node_t *rb_maximum(rb_node_t *t);
RBTREE_DECL rb_node_t *rb_successor(rb_node_t *t);
RBTREE_DECL rb_node_t *rb_predecessor(rb_node_t *t);

#endif /* RBTREE_H */

//#define RBTREE_IMPLEMENTATION
#ifdef RBTREE_IMPLEMENTATION

static rb_node_t rb_nil = { &rb_nil, &rb_nil, &rb_nil, RB_BLACK };

static inline void i_rb_insert_bst(rb_tree_t *t, rb_node_t *node)
{
	if (t == NULL || node == NULL) return;

	rb_node_t *curr = t->root;
	rb_node_t *prnt = &rb_nil;

	while (curr != &rb_nil) {
		prnt = curr;

		if (t->cmp(node, curr) <= 0)
			curr = curr->left;
		else
			curr = curr->right;
	}

	node->parent = prnt;
	node->left   = &rb_nil;
	node->right  = &rb_nil;
	node->color  = RB_RED;

	if (prnt == &rb_nil)
		t->root = node;
	else if (t->cmp(node, prnt) <= 0)
		prnt->left = node;
	else
		prnt->right = node;
}

static inline void i_rb_rotl(rb_node_t **x)
{
	rb_node_t *old = *x;
	rb_node_t *y   = old->right;
	rb_node_t *b   = y->left;
	
	old->right = b;
	b->parent  = old;

	y->left     = old;
	y->parent   = old->parent;
	old->parent = y;

	*x = y;
}

static inline void i_rb_rotr(rb_node_t **y)
{
	rb_node_t *old = *y;
	rb_node_t *x   = old->left;
	rb_node_t *b   = x->right;

	old->left = b;
	b->parent = old;

	x->right    = old;
	x->parent   = old->parent;
	old->parent = x;

	*y = x;
}

static inline rb_node_t **i_rb_node_slot(rb_tree_t *t, rb_node_t *n)
{
	if (n->parent == &rb_nil) return &t->root;
	if (n->parent->left == n) return &n->parent->left;
	return &n->parent->right;
}

static inline void i_rb_insert_fixup(rb_tree_t *t, rb_node_t *z)
{
	while (z->parent->color == RB_RED) {
		rb_node_t *p = z->parent;
		rb_node_t *g = p->parent;

		if (p == g->left) {
			rb_node_t *u = g->right;

			if (u->color == RB_RED) {
				p->color = RB_BLACK;
				u->color = RB_BLACK;
				g->color = RB_RED;
				z = g;
			} else {
				if (z == p->right) {
					z = p;
					i_rb_rotl(i_rb_node_slot(t, z));
				}

				z->parent->color = RB_BLACK;
				z->parent->parent->color = RB_RED;
				i_rb_rotr(i_rb_node_slot(t, z->parent->parent));
			}
		} else {
			rb_node_t *u = g->left;

			if (u->color == RB_RED) {
				p->color = RB_BLACK;
				u->color = RB_BLACK;
				g->color = RB_RED;
				z = g;
			} else {
				if (z == p->left) {
					z = p;
					i_rb_rotr(i_rb_node_slot(t, z));
				}

				z->parent->color = RB_BLACK;
				z->parent->parent->color = RB_RED;
				i_rb_rotl(i_rb_node_slot(t, z->parent->parent));
			}
		}
	}

	t->root->color = RB_BLACK;
}

static inline void i_rb_transplant(rb_tree_t *t, rb_node_t *u, rb_node_t *v)
{
	if (u->parent == &rb_nil)
		t->root = v;
	else if (u == u->parent->left)
		u->parent->left = v;
	else
	 	u->parent->right = v;

	v->parent = u->parent;
}

static void i_rb_delete_fixup(rb_tree_t *t, rb_node_t *x)
{
	while (x != t->root && x->color == RB_BLACK) {
		if (x == x->parent->left) {
			rb_node_t *s = x->parent->right;

			if (s->color == RB_RED) {
				s->color         = RB_BLACK;
				x->parent->color = RB_RED;
				i_rb_rotl(i_rb_node_slot(t, x->parent));
				s = x->parent->right;
			}

			if (s->right->color == RB_BLACK && s->left->color == RB_BLACK) {
				s->color = RB_RED;
				x = x->parent;
			} else {
				if (s->right->color == RB_BLACK) {
					s->color       = RB_RED;
					s->left->color = RB_BLACK;
					i_rb_rotr(i_rb_node_slot(t, s));
					s = x->parent->right;
				}

				s->color         = x->parent->color;
				x->parent->color = RB_BLACK;
				s->right->color  = RB_BLACK;
				i_rb_rotl(i_rb_node_slot(t, x->parent));
				x = t->root;
			}
		} else {
			rb_node_t *s = x->parent->left;

			if (s->color == RB_RED) {
				s->color         = RB_BLACK;
				x->parent->color = RB_RED;
				i_rb_rotr(i_rb_node_slot(t, x->parent));
				s = x->parent->left;
			}

			if (s->right->color == RB_BLACK && s->left->color == RB_BLACK) {
				s->color = RB_RED;
				x = x->parent;
			} else {
				if (s->left->color == RB_BLACK) {
					s->color       = RB_RED;
					s->right->color = RB_BLACK;
					i_rb_rotl(i_rb_node_slot(t, s));
					s = x->parent->left;
				}

				s->color         = x->parent->color;
				x->parent->color = RB_BLACK;
				s->left->color  = RB_BLACK;
				i_rb_rotr(i_rb_node_slot(t, x->parent));
				x = t->root;
			}
		}
	}

	x->color = RB_BLACK;
}

RBTREE_DECL void rb_init(rb_tree_t *t, rb_cmp_t cmp, rb_key_cmp_t kcmp)
{
	t->root = &rb_nil;
	t->cmp  = cmp;
	t->kcmp = kcmp;
}

RBTREE_DECL void rb_insert(rb_tree_t *t, rb_node_t *node)
{
	i_rb_insert_bst(t, node);
	i_rb_insert_fixup(t, node);
}

RBTREE_DECL void rb_delete(rb_tree_t *t, rb_node_t *node)
{
	if (t == NULL || node == NULL || node == &rb_nil) return;

	rb_node_t *z = node;
	rb_node_t *y = z;
	rb_node_t *x;

	rb_color_t y_orig_color = y->color;

	if (z->left == &rb_nil) {
		x = z->right;
		i_rb_transplant(t, z, x);
	} else if (z->right == &rb_nil) {
		x = z->left;
		i_rb_transplant(t, z, x);
	} else {
		y = rb_minimum(z->right);
		y_orig_color = y->color;
		x = y->right;

		if (y->parent == z) {
			x->parent = y;
		} else {
			i_rb_transplant(t, y, y->right);
			y->right = z->right;
			y->right->parent = y;
		}

		i_rb_transplant(t, z, y);
		y->left = z->left;
		y->left->parent = y;
		y->color = z->color;
	}

	if (y_orig_color == RB_BLACK)
		i_rb_delete_fixup(t, x);
}

RBTREE_DECL rb_node_t *rb_search(const rb_tree_t *t, const void *key)
{
	if (t == NULL || key == NULL) return NULL;

	rb_node_t *curr = t->root;
	int comp;

	while (curr != &rb_nil && (comp = t->kcmp(key, curr)) != 0) {
		if (comp < 0)
			curr = curr->left;
		else
			curr = curr->right;
	}

	return curr;
}

RBTREE_DECL rb_node_t *rb_minimum(rb_node_t *t)
{
	if (t == NULL || t == &rb_nil) return NULL;

	while (t->left != &rb_nil) t = t->left;

	return t;
}

RBTREE_DECL rb_node_t *rb_maximum(rb_node_t *t)
{
	if (t == NULL || t == &rb_nil) return NULL;

	while (t->right != &rb_nil) t = t->right;

	return t;
}

RBTREE_DECL rb_node_t *rb_successor(rb_node_t *node)
{
	if (node == NULL || node == &rb_nil) return NULL;
	if (node->right != &rb_nil) return rb_minimum(node->right);
	
	rb_node_t *curr = node;
	rb_node_t *prnt = node->parent;

	while (prnt != &rb_nil && curr == prnt->right) {
		curr = prnt;
		prnt = prnt->parent;
	}

	return prnt == &rb_nil ? NULL : prnt;
}

RBTREE_DECL rb_node_t *rb_predecessor(rb_node_t *node)
{
	if (node == NULL || node == &rb_nil) return NULL;
	if (node->left != &rb_nil) return rb_maximum(node->left);

	rb_node_t *curr = node;
	rb_node_t *prnt = node->parent;

	while (prnt != &rb_nil && curr == prnt->left) {
		curr = prnt;
		prnt = prnt->parent;
	}

	return prnt == &rb_nil ? NULL : prnt;
}

#endif /* RBTREE_IMPLEMENTATION */

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