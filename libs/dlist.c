#include <aoc/dlist.h>
#include <assert.h>

void aoc_dlist_init(struct aoc_dlist_node *dlist)
{
	assert(dlist != NULL);
	dlist->prev = dlist;
	dlist->next = dlist;
	return;
}

/* insert "node" between "before" and "after" */
static void aoc_dlist_insert(struct aoc_dlist_node *before,
	struct aoc_dlist_node *node, struct aoc_dlist_node *after)
{
	before->next = node;
	node->prev = before;
	node->next = after;
	after->prev = node;
	return;
}

void aoc_dlist_append(struct aoc_dlist_node *dlist, 
	struct aoc_dlist_node *node)
{
	assert(dlist != NULL);
	assert(node != NULL);
	aoc_dlist_insert(dlist, node, dlist->next);
	return;
}

void aoc_dlist_prepend(struct aoc_dlist_node *dlist,
	struct aoc_dlist_node *node)
{
	assert(dlist != NULL);
	assert(node != NULL);
	aoc_dlist_insert(dlist->prev, node, dlist);
	return;
}

/* remove "node" from the middle of "before" and "after" */
static void aoc_dlist_remove_node(struct aoc_dlist_node *before,
	struct aoc_dlist_node *node, struct aoc_dlist_node *after)
{
	before->next = after;
	after->prev = before;
	node->prev = NULL;
	node->next = NULL;
	return;
}

void aoc_dlist_remove(struct aoc_dlist_node *node)
{
	assert(node != NULL);
	aoc_dlist_remove_node(node->prev, node, node->next);
	return;
}

void *aoc_dlist_container(struct aoc_dlist_node *node, size_t offset)
{
	assert(node != NULL);
	char *container = (char *)node;
	return container - offset;
}

