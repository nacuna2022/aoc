#include <aoc/map.h>
#include <aoc/incache.h>

#include <stdlib.h>

struct aoc_map {
	int line_lize;
	int size;
	char data[];
};

static struct aoc_map *aoc_map_init(struct aoc_map *map,
		struct aoc_incache *incache)
{
	/* basically just iterate over incache to get all characters
	 * skipping over all newlines */

	return map;
}

static struct aoc_map *aoc_map_new(struct aoc_incache *incache)
{
	struct aoc_map *map;
	size_t size;
	if (incache == NULL)
		return NULL;

	size = aoc_incache_size(incache);
	map = malloc((sizeof * map) + size);
	if (map == NULL)
		return NULL;

	return aoc_map_init(map, incache);
}

struct aoc_map *aoc_new_map(char *pathname)
{
	struct aoc_incache *incache;
	incache = aoc_new_incache(pathname);
	return aoc_map_new(incache);
}

void aoc_free_map(struct aoc_map *map)
{
	return;
}

int aoc_map_peek_up(struct aoc_map *map, char *tile)
{
	return -1;
}

int aoc_map_peek_right(struct aoc_map *map, char *tile)
{
	return -1;
}

int aoc_map_peek_down(struct aoc_map *map, char *tile)
{
	return -1;
}

int aoc_map_peek_left(struct aoc_map *map, char *tile)
{
	return -1;
}

int aoc_map_go_up(struct aoc_map *map, char *tile)
{
	return -1;
}

int aoc_map_go_right(struct aoc_map *map, char *tile)
{
	return -1;
}

int aoc_map_go_down(struct aoc_map *map, char *tile)
{
	return -1;
}

int aoc_map_go_left(struct aoc_map *map, char *tile)
{
	return -1;
}

