#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include <aoc/lncache.h>
#include <aoc/incache.h>
#include <aoc/dlist.h>

struct aoc_lncache {
	struct aoc_dlist_node node;
};

struct aoc_line {
	struct aoc_dlist_node node;
	size_t count;
	size_t size;
	char data[];
};

struct aoc_lncache *aoc_new_lncache(char *pathname)
{
	struct aoc_incache *incache;
	struct aoc_lncache *lncache;
	struct aoc_line *line = NULL;
	int idx;
	static const size_t resize = 32;
	
	incache = aoc_new_incache(pathname);
	if (incache == NULL) {
		return NULL;
	}

	lncache = malloc(sizeof * lncache);
	if (lncache == NULL) {
		goto release_incache;
	}
	aoc_dlist_init(&lncache->node);

	idx = -1;
	for(;;) {
		int ch;
		idx += 1;
		ch = aoc_incache_get(incache, idx);
		if (ch == -1)
			break;

		if (ch == '\n') {
			if (line != NULL) {
				aoc_dlist_prepend(&lncache->node, &line->node);
			}
			line = NULL;
			continue;
		}
		
		if (line == NULL) {
			line = malloc(sizeof * line);
			if (line == NULL) {
				aoc_free_lncache(lncache);
				lncache = NULL;
				goto release_incache;
			}
			line->count = 0;
			line->size = 0;
		}

		while (line->count >= line->size) {
			struct aoc_line *tmp;
			tmp = realloc(line, 
				(sizeof * line) + line->size + resize);
			if (tmp == NULL) {
				aoc_free_lncache(lncache);
				lncache = NULL;
				goto release_incache;
			}
			tmp->size += resize;
			line = tmp;
		}

		line->data[line->count] = (char)ch;
		line->count += 1;
	}

release_incache:
	aoc_free_incache(incache);
	return lncache;
}

void aoc_free_lncache(struct aoc_lncache *lncache)
{
	struct aoc_dlist_node *ptr;
	for (ptr = lncache->node.next; ptr != &lncache->node; ) {
		struct aoc_dlist_node *next = ptr->next;
		free(ptr);
		ptr = next;
	}
	free(lncache);
	return;
}

void aoc_lncache_print_line(struct aoc_line *line)
{
	size_t i;
	assert(line != NULL);
	for(i = 0; i < line->count; i++) {
		putchar(line->data[i]);
	}
	putchar('\n');
	return;
}

void aoc_lncache_print(struct aoc_lncache *lncache)
{
	struct aoc_dlist_node *ptr;
	for (ptr = lncache->node.next; ptr != &lncache->node;
		ptr = ptr->next) {
		struct aoc_line *line;
		line = aoc_dlist_container(ptr, offsetof(struct aoc_line, node));
		aoc_lncache_print_line(line);
	}
	return;
}

