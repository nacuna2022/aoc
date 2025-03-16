#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>

struct antenna {
	uintptr_t location;
	int frequency;
	struct antenna *next;
};

struct tile_map {
	char *buffer;
	size_t size;
	size_t line_size;
	struct antenna *antinode_list;
	struct antenna *antenna_lut[CHAR_MAX];
};

static void init_tile_map(struct tile_map *map, char *pathname)
{
	struct stat statbuf;
	char *buffer;
	size_t map_size;
	size_t i;
	FILE *input;
	int ch;

	if (stat("input", &statbuf) == -1) {
		fprintf(stderr, "cannot stat input\n");
		exit(-1);
	}
	buffer = malloc(statbuf.st_size);
	assert(buffer != NULL);

	input = fopen(pathname, "r");
	assert(input != NULL);
	map_size = 0;
	map->line_size = 0;
	while((ch = fgetc(input)) != EOF) {
		if (ch != '\n') {
			*(buffer + map_size) = (char)ch;
			map_size += 1;
		}

		if (ch == '\n' && (map->line_size == 0)) {
			map->line_size = map_size;
		}
	}
	fclose(input);
	map->buffer = buffer;
	map->size = map_size;
	map->antinode_list = NULL;
	for (i = 0; i < CHAR_MAX; i++) {
		map->antenna_lut[i] = NULL;
	}
	return;
}

static struct tile_map *new_tile_map_from_input(char *pathname)
{
	struct tile_map *map;
	map = malloc(sizeof * map);
	init_tile_map(map, pathname);
	assert(map != NULL);
	return map;
}

static void new_antenna(struct antenna **ant_pp, uintptr_t location, int freq)
{
	struct antenna *ant;
	ant = malloc(sizeof * ant);
	assert(ant != NULL);

	ant->location = location;
	ant->frequency = freq;
	ant->next = NULL;
	*ant_pp = ant;
	return;
}

static void antenna_new(struct tile_map *map, char *location)
{
	struct antenna **ant_pp;
	int freq = (*location) & 0xFF;
	
	assert((freq > 0) && (freq < CHAR_MAX));
	ant_pp = &(map->antenna_lut[freq]);
	for (;;) {
		struct antenna *ant_ptr = *ant_pp;
		if (ant_ptr == NULL) {
			break;
		}
		assert(ant_ptr->location != (uintptr_t)location);
		assert(ant_ptr->frequency == freq);
		ant_pp = &(ant_ptr->next);
	}
	new_antenna(ant_pp, (uintptr_t)location, freq);
	return;
}

/* so apparently, every other character is a frequency except
 * the dot character which denotes an empty tile. */
static inline bool is_antenna_tile(char *location)
{
	return *location != '.';
}

static void scan_antenna_field(struct tile_map *map)
{
	char *tile_ptr;
	tile_ptr = map->buffer;
	while(tile_ptr < (map->buffer + map->size)) {
		if (is_antenna_tile(tile_ptr)) {
			antenna_new(map, tile_ptr);
		}
		tile_ptr += 1;
	}
	return;
}

struct coord {
	int x;
	int y;
};

static inline void get_coord(struct tile_map *map, uintptr_t loc,
		struct coord *coord)
{
	uintptr_t offset = loc - (uintptr_t)map->buffer;
	coord->x = offset / map->line_size;
	coord->y = offset % map->line_size;
	return;
}

static void print_tile_map(struct tile_map *map)
{
	char *p = map->buffer;
	while(p < (map->buffer + map->size)) {
		size_t i;
		for (i = 0; i < map->line_size; i++, p++) {
			printf("%c", *p);
		}
		printf("\n");
	}
	printf("\n");
	return;
}

static void alloc_antinode(struct antenna **antpp, uintptr_t location,
		int frequency)
{
	struct antenna *ant;
	ant = malloc(sizeof *ant);
	assert(ant != NULL);
	ant->location = location;
	ant->frequency = frequency;
	ant->next = NULL;
	*antpp = ant;
	return;
}

static bool new_antinode_at_location(struct tile_map *map, uintptr_t location, 
		int frequency)
{
	struct antenna **antinode;

	/* check if we already have an antinode at location */
	for (antinode = &map->antinode_list; *antinode != NULL; antinode = &(*antinode)->next) {
		struct antenna *ant = *antinode;
		if (ant->location == location) {
			return false;
		}
	}

	/* if we reach here, this is a unique node */
	alloc_antinode(antinode, location, frequency);
	
	return true;
}

/* always install antinode with reference to ant1 */
static int install_antinode(struct tile_map *map, struct antenna *ant1, 
		struct antenna *ant2, intptr_t distance, int freq)
{
	char *new_location;
	int delta1;
	int delta2;
	int count = 0;

	/* distance cannot be zero! */
	assert(distance != 0);
	new_location = (char *)((intptr_t)ant1->location + distance);

	/* check if we are stepping off the starting and ending points 
	 * of the map. */
	if (new_location < map->buffer)
		return 0;
	if (new_location > ((map->buffer + map->size)-1))
		return 0;

	/* if we reach here, we are still within the bounds of the start/end of 
	 * the map. but things are more nuanced than that. so we do more 
	 * checks. */
	struct coord an1_coord;
	struct coord an2_coord;
	struct coord antinode_coord;

	get_coord(map, ant1->location, &an1_coord);
	get_coord(map, ant2->location, &an2_coord);
	get_coord(map, (uintptr_t)new_location, &antinode_coord);

	delta1 = abs(an1_coord.x - an2_coord.x);
	delta2 = abs(an1_coord.x - antinode_coord.x);
	if (delta1 != delta2)
		return 0;

	delta1 = abs(an1_coord.y - an2_coord.y);
	delta2 = abs(an1_coord.y - antinode_coord.y);
	if (delta1 != delta2)
		return 0;

	struct antenna ant_antinode = {
		.location = (uintptr_t)new_location,
	};

	if (new_antinode_at_location(map, (uintptr_t)new_location, freq) == true) {
		count += 1;
	}

	if (new_antinode_at_location(map, ant2->location, freq) == true) {
		count += 1;
	}

	return count + install_antinode(map, &ant_antinode, ant1, distance, freq);
}

#define for_each_antenna(ant, list) \
	for ((ant) = (list); (ant) != NULL; (ant) = (ant)->next) 

static int discover_by_antenna_list(struct tile_map *map,
		struct antenna *ant_list)
{
	struct antenna *ant1;
	struct antenna *ant2;
	intptr_t distance;
	int count = 0;

	for_each_antenna(ant1, ant_list) {
		for_each_antenna(ant2, ant_list) {
			if (ant1 == ant2)
				continue;
		
			if (new_antinode_at_location(map, (uintptr_t)ant1->location, ant1->frequency) == true) {
				count += 1;
			}

			/* if distance is negative: ant1 comes before ant2.
			 * we need to install antinode _before_ ant1.
			 *
			 * if distance is positive: ant1 comes after ant2.
			 * we need to install antinode _after_ ant1.
			 */
			distance = (intptr_t)(ant1->location) - (intptr_t)(ant2->location);
			count += install_antinode(map, ant1, ant2, distance, ant1->frequency);
		}
	}

	return count;
}

static int discover_all_antinodes(struct tile_map *map)
{
	size_t i;
	int count = 0;
	for (i = 0; i < CHAR_MAX; i++) {
		struct antenna *ant_list = map->antenna_lut[i];
		if (ant_list == NULL) {
			continue;
		}

		/* skip single antennas */
		if (ant_list->next == NULL) {
			continue;
		}

		count += discover_by_antenna_list(map, ant_list);
	} 
	return count; 
}

void free_antenna_list(struct antenna *list)
{
	struct antenna *ant;
	struct antenna *tmp;
	for (ant = list; ant != NULL; ) {
		tmp = ant->next;
		free(ant);
		ant = tmp;
	}
	return;
}

static void free_tile_map(struct tile_map *map)
{
	size_t i;
	struct antenna *ant;
	struct antenna *tmp;
	for (i = 0; i < CHAR_MAX; i++) {
		free_antenna_list(map->antenna_lut[i]);
	}
	free_antenna_list(map->antinode_list);
	free(map->buffer);
	free(map);
	return;
}

int main(void)
{
	int antinode_count;
	struct tile_map *antenna_field;
	antenna_field = new_tile_map_from_input("input");
	scan_antenna_field(antenna_field);
	antinode_count = discover_all_antinodes(antenna_field);
	printf("unique antinode: %d\n", antinode_count);
	free_tile_map(antenna_field);
	return 0;
}

