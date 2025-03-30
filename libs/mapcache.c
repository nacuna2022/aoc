#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include <aoc/mapcache.h>

struct aoc_mapcache {
	int linesize;
	int size;
	char *pos;
	char data[];
};

static void init_mapcache(struct aoc_mapcache *cache,
	struct aoc_incache *incache)
{
	bool linesize_found = false;
	int size = 0;
	int linesize;
	int ch;
	int idx = 0;
	while((ch = aoc_incache_get(incache, idx++)) != -1) {
		if (ch == '\n') {
			if (!linesize_found) {
				linesize = size;
				linesize_found = true;
			}
			continue;
		}
		cache->data[size] = (char)ch;
		size += 1;
	}
	cache->size = size;
	cache->pos = &cache->data[0];
	return;
}

struct aoc_mapcache *aoc_new_mapcache(char *pathname)
{
	struct aoc_incache *incache;
	struct aoc_mapcache *cache;
	size_t rawsize;
	incache = aoc_new_incache(pathname);
	if (incache == NULL)
		return NULL;
	rawsize = aoc_incache_size(incache);
	cache = malloc(sizeof(struct aoc_mapcache) + rawsize);
	if (cache != NULL) {
		init_mapcache(cache, incache);
	}
	aoc_free_incache(incache);
	return cache;
}

void aoc_free_mapcache(struct aoc_mapcache *cache)
{
	assert(cache != NULL);
	free(cache);
	return;
}

static bool out_of_bounds(struct aoc_mapcache *cache, char *pos)
{
	char *start = cache->data;
	char *end = cache->data + cache->size;
	if ((pos < start) || (pos > (end-1))) {
		return true;
	} 
	return false;
}

static char *new_pos(struct aoc_mapcache *cache, int offset)
{
	char *ptr = cache->pos + offset;
	if (out_of_bounds(cache, ptr)) {
		return NULL;
	}
	return ptr;
}

static int walk(struct aoc_mapcache *cache, int offset)
{
	char *ptr;
	int ch = -1;
	ptr = new_pos(cache, offset);
	if (ptr != NULL) {
		cache->pos = ptr;
		ch = aoc_mapcache_tile(cache, NULL);
	}
	return ch;
}

int aoc_mapcache_walk_forward(struct aoc_mapcache *cache)
{
	assert(cache != NULL);
	return walk(cache, 1);
}

int aoc_mapcache_walk_backward(struct aoc_mapcache *cache)
{
	assert(cache != NULL);
	return walk(cache, -1);
}

int aoc_mapcache_tile(struct aoc_mapcache *cache, unsigned long *tile_id)
{
	assert(cache != NULL);
	if (tile_id) {
		*tile_id = (unsigned long)cache->pos;
	}
	return (int)(*cache->pos & 0xFF);
}

