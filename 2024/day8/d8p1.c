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


/* always install antinode with reference to ant1 */
static bool install_antinode(struct tile_map *map, struct antenna *ant1, 
		struct antenna *ant2, intptr_t distance)
{
	char *new_location;
	int delta1;
	int delta2;

	/* distance cannot be zero! */
	assert(distance != 0);
	new_location = (char *)((intptr_t)ant1->location + distance);

	/* check if we are stepping off the starting and ending points 
	 * of the map. */
	if (new_location < map->buffer)
		return false;
	if (new_location > ((map->buffer + map->size)-1))
		return false;

	/* if we reach here, we are still within the bounds of the start/end of 
	 * the map. but things are more nuanced than that. so we do more 
	 * checks. */
	struct coord an1_coord;
	struct coord an2_coord;
	struct coord antinode_coord;

	get_coord(map, (uintptr_t)ant1->location, &an1_coord);
	get_coord(map, (uintptr_t)ant2->location, &an2_coord);
	get_coord(map, (uintptr_t)new_location, &antinode_coord);

	delta1 = abs(an1_coord.x - an2_coord.x);
	delta2 = abs(an1_coord.x - antinode_coord.x);
	if (delta1 != delta2)
		return false;

	delta1 = abs(an1_coord.y - an2_coord.y);
	delta2 = abs(an1_coord.y - antinode_coord.y);
	if (delta1 != delta2)
		return false;

	/* install the antinode on this tile - overwriting the antenna. */
	if (*new_location == '#')
		return false;
	*new_location = '#';

	return true;
}

static void print_tile_map(struct tile_map *map)
{
	char *p = map->buffer;
	while(p < (map->buffer + map->size)) {
		size_t i;
		/* nacuna??? revisit this! */
		for (i = 0; i < map->line_size; i++, p++) {
			printf("%c", *p);
		}
		printf("\n");
	}
	printf("\n");
	return;
}


#define for_each_antenna(ant, list) \
	for ((ant) = (list); (ant) != NULL; (ant) = (ant)->next) 

static int discover_by_antenna_list(struct tile_map *map,
		struct antenna *ant_list)
{
	int antinode_count = 0;
	struct antenna *ant1;
	struct antenna *ant2;
	intptr_t distance;

	for_each_antenna(ant1, ant_list) {
		for_each_antenna(ant2, ant_list) {
			bool install_ok;
			if (ant1 == ant2)
				continue;

			/* if distance is negative: ant1 comes before ant2.
			 * we need to install antinode _before_ ant1.
			 *
			 * if distance is positive: ant1 comes after ant2.
			 * we need to install antinode _after_ ant1.
			 */
			distance = (intptr_t)(ant1->location) - (intptr_t)(ant2->location);
			install_ok = install_antinode(map, ant1, ant2,
					distance);
			antinode_count += (install_ok == true ? 1 : 0);
		}
	}

	return antinode_count;
}

static void discover_all_antinodes(struct tile_map *map)
{
	size_t i;
	int total = 0; 
	for (i = 0; i < CHAR_MAX; i++) {
		struct antenna *ant_list = map->antenna_lut[i];
		if (ant_list == NULL) {
			continue;
		}
		total += discover_by_antenna_list(map,
				ant_list);
	} 
	printf("unique antinodes: %d\n", total);
	return; 
}

static void free_tile_map(struct tile_map *map)
{
	size_t i;
	for (i = 0; i < CHAR_MAX; i++) {
		free(map->antenna_lut[i]);
	}
	free(map->buffer);
	free(map);
	return;
}

int main(void)
{
	struct tile_map *antenna_field;
	antenna_field = new_tile_map_from_input("input");
	scan_antenna_field(antenna_field);
	discover_all_antinodes(antenna_field);
	free_tile_map(antenna_field);
	return 0;
}

