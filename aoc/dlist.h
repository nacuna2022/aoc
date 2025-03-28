#ifndef __AOC_DLIST_H__
#define __AOC_DLIST_H__
#include <stddef.h>

/* doubly linked list implementation */

struct aoc_dlist_node {
	struct aoc_dlist_node *prev;
	struct aoc_dlist_node *next;
};

void aoc_dlist_init(struct aoc_dlist_node *dlist_node);
void aoc_dlist_append(struct aoc_dlist_node *dlist, 
	struct aoc_dlist_node *node);
void aoc_dlist_prepend(struct aoc_dlist_node *dlist,
	struct aoc_dlist_node *node);
void aoc_dlist_remove(struct aoc_dlist_node *node);
void *aoc_dlist_container(struct aoc_dlist_node *node, size_t offset);

#endif /* __AOC_DLIST_H__ */
