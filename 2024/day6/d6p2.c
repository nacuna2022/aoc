#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

enum cardinal_direction {
	DIR_NORTH = 0, 	/* up */
	DIR_EAST,	/* right */
	DIR_SOUTH,	/* down */
	DIR_WEST,	/* left */
	DIR_MAX,
};

static struct guard_struct {
	char *location;
	enum cardinal_direction facing;
	bool initialized;
	char *location_copy;
	enum cardinal_direction facing_copy;
} guard = { .initialized = false, };

/* we record (on certain points in the map) information about the direction
 * the guard took when it was last in that tile.
 */
static struct tile_info {
#define TILE_HASH_SIZE_SHIFT	7
#define TILE_HASH_SIZE		(1 << (TILE_HASH_SIZE_SHIFT))
	uintptr_t location; /* memory address are our keys */
	enum cardinal_direction direction;
	struct tile_info *next; /* hash table chaining */
	struct tile_info *next_list; /* linked list of all tile_info objects */
} *tile_info_ht[TILE_HASH_SIZE] = {0};

static char *tile_map = NULL;
static char *tile_map_copy = NULL;
static int tile_size = 0;
static int tile_line_size = 0;
static struct tile_info *tile_info_head = NULL;

static void reset_hash_table(void)
{
	size_t i;

	/* first free all the tile_info we allocated */
	while (tile_info_head != NULL) {
		struct tile_info *next = tile_info_head->next_list;
		free(tile_info_head);
		tile_info_head = next;
	}

	/* now we reset the hash table */
	for (i = 0; i < TILE_HASH_SIZE; i++) {
		tile_info_ht[i] = NULL;
	}
	return;
}

static struct tile_info *alloc_tile_info(struct tile_info **pp, 
		uintptr_t location)
{
	struct tile_info *p;
	if ((p = malloc(sizeof *p))) {
		p->location = location;
		p->direction = DIR_MAX; /* undefined direction _for_now_ */
		p->next = NULL;
		p->next_list = NULL;
	}

	p->next_list = tile_info_head;
	tile_info_head = p;
	*pp = p;
	return p;
}

static unsigned long fnv1a_hash(char *data, size_t size)
{
	size_t i;
	unsigned char *pdata = (unsigned char *)data;
	static const unsigned long fnv1a_offset = 0xcbf29ce484222325;
	static const unsigned long fnv1a_prime = 0x100000001b3;
	unsigned long h;
	h = fnv1a_offset;
	for (i = 0; i < size; i++) {
		h = h ^ pdata[i];
		h = h * fnv1a_prime;
	}
	return h;
}

static unsigned long location2hash_idx(char *location)
{
	unsigned long h;
	h = fnv1a_hash(location, sizeof location);

	/* we should XOR-fold at this point, but lets just truncate the 
	 * hash based on the hash table size */
	return h & (TILE_HASH_SIZE-1);
}

static struct tile_info *tile_info_alloc(char *location)
{
	unsigned long h;
	uintptr_t address; 

	h = location2hash_idx(location);
	struct tile_info **pp = &tile_info_ht[h];

	for(;;) {
		struct tile_info *p = *pp;
		address = (uintptr_t)location;
		if (!p)
			break;

		if (address == p->location)
			return p;

		pp = &(p->next);
	}

	return alloc_tile_info(pp, address);
}

static struct tile_info *get_tile_info(char *location)
{
	unsigned long h;
	h = location2hash_idx(location);

	struct tile_info **pp = &tile_info_ht[h];
	for (;;) {
		struct tile_info *p = *pp;
		uintptr_t address = (uintptr_t)location;
		if (!p)
			break;
		if (address == p->location)
			return p;

		pp = &(p->next);
	}

	return NULL;
}

/* these are absolute directions not relative to guard */
static char *get_tile_from_direction(char *tile_ptr, 
		enum cardinal_direction dir)
{
	char *new_ptr;
	char *line_end;
	char *tile_map_end = tile_map + tile_size;
	int idx;
	switch(dir) {
	case DIR_NORTH:
	{
		new_ptr = tile_ptr - tile_line_size;
		if (new_ptr >= tile_map) {
			return new_ptr;
		}
		break;
	}
	case DIR_EAST:
	{
		/* find the line ending index so we know if we are walking off
		 * the cliff when we step east. line_round_up() */
		idx = (int)(tile_ptr - tile_map);
		idx = ((idx + (tile_line_size - 1)) / tile_line_size) * tile_line_size;
		new_ptr = tile_ptr + 1;
		if (new_ptr < (tile_map + idx)) {
			return new_ptr;
		}
		break;
	}
	case DIR_SOUTH:
	{
		new_ptr = tile_ptr + tile_line_size;
		if (new_ptr < tile_map_end) {
			return new_ptr;
		}
		break;
	}
	case DIR_WEST:
	{
		/* see DIR_EAST. line_round_down() */
		idx = (int)(tile_ptr - tile_map);
		idx = (idx / tile_line_size) * tile_line_size;
		new_ptr = tile_ptr - 1;
		if (new_ptr >= (tile_map + idx)) {
			return new_ptr;
		}
		break;
	}
	default:
		assert(0);
	}
	return NULL;
}

/* peek to see if there is a blocker in path */
static char guard_peek(struct guard_struct *guard, 
		enum cardinal_direction direction)
{
	char *tile_ptr;
	tile_ptr = get_tile_from_direction(guard->location, direction);
	if (tile_ptr) {
		return *tile_ptr;
	}
	return 0;
}

static char guard_peek_front(struct guard_struct *guard)
{
	return guard_peek(guard, guard->facing);
}

static char guard_peek_right(struct guard_struct *guard)
{
	return guard_peek(guard, (guard->facing + 1) % DIR_MAX);
}

/* we step on the direction where guard is facing currently.
 * '#' means guard can turn right.
 * '@' means guard needs to turn right twice.. e.g. go the opposite direction
 * 0 means guards has stepped off the map */
static bool guard_step(struct guard_struct *guard, char *reason)
{
	char front;
	struct tile_info *tile_info;
	front = guard_peek_front(guard);
	if ((front != 0) && (front != '#')) {
		guard->location = get_tile_from_direction(guard->location,
				guard->facing);
		return true;
	}

	*reason = front;
	if (front == '#') {
		char right = guard_peek_right(guard);
		if (right == '#') {
			*reason = '@';
		}
	}
	return false;
}

static void guard_turn(struct guard_struct *guard, int i)
{
	enum cardinal_direction new_facing = (guard->facing + i) % DIR_MAX;
	guard->facing = new_facing;
	return;
}

static void guard_turn_right(struct guard_struct *guard)
{
	guard_turn(guard, 1);
	return;
}

static void init_guard(struct guard_struct *guard, char *location,
		enum cardinal_direction facing)
{
	if (guard->initialized == false) {
		guard->location = location;
		guard->facing = facing;
		guard->location_copy = location;
		guard->facing_copy = facing;
		guard->initialized = true;
	}
	return;
}

static void reset_guard(struct guard_struct *guard)
{
	assert(guard->initialized == true);
	guard->initialized = false;
	init_guard(guard, guard->location_copy, guard->facing_copy);
	return;
}

static inline char *guard_home(struct guard_struct *guard)
{
	return guard->location_copy;
}

static void print_tile_map(void)
{
	int i;
	char *ptr = tile_map;
	while(*ptr != '\0') {
		for (i=0; i<tile_line_size; i++) {
			printf("%c", *ptr);
			ptr++;
		}
		printf("\n");
	}
	return;
}

static void guard_mark_position(struct guard_struct *guard)
{
	char *tile_ptr = guard->location;
	if (*tile_ptr != 'X') {
		*tile_ptr = 'X';
	}
	return;
}

int main(void)
{
	struct stat input_stat;
	int input_fd;
	int err;
	char ch;
	char *guard_loc_ptr = NULL;
	size_t newline_count = 0;
	int loop_count = 0;
	
	if (stat("input", &input_stat) == -1) {
		fprintf(stderr, "cannot stat input file\n");
		exit(-1);
	}

	/* we now know how big of a buffer to allocate. this buffer is a little
	 * bit bigger than what we actually want because each line in input 
	 * file has a terminating newline ('\n') which fstat also counts. 
	 */
	if ((tile_map = malloc(input_stat.st_size)) == NULL) {
		fprintf(stderr, "cannot allocate tile map\n");
		exit(-1);
	}

	input_fd = open("input", O_RDONLY);
	while((err = read(input_fd, &ch, 1)) != 0) {
		if (err == -1) {
			fprintf(stderr, "error reading from input\n");
			exit(-1);
		}

		assert((ch == '.') || (ch == '#') || (ch == '^') || (ch == '\n'));

		if (ch == '\n') {
			newline_count += 1;
			continue;
		}

		/* additionally, lets record the location of the guard 
		 * on the tile map. assumption is that the guard always 
		 * initially faces the north direction
		 */
		if (ch == '^') {
			assert(guard_loc_ptr == NULL);
			guard_loc_ptr = tile_map + tile_size;
			init_guard(&guard, guard_loc_ptr, DIR_NORTH);
			ch = '.';
		}

		*(tile_map + tile_size) = ch;

		tile_size += 1;
	}

	tile_map_copy = malloc(tile_size);
	memcpy(tile_map_copy, tile_map, tile_size);
	
	/* make sure we have a guard facing the up direction */
	assert(guard_loc_ptr != NULL);
	tile_line_size = tile_size / newline_count;

	guard_mark_position(&guard);
	char *tile_i;
	for (tile_i = tile_map; tile_i < (tile_map + tile_size); tile_i++) {
		if (*tile_i == '#') 
			continue;

		if (tile_i == guard_home(&guard))
			continue;

		/* put a blocker on this tile */
		*tile_i = '#';
		//printf("New tile map\n");
		//print_tile_map();
		//printf("\n");

		/* start simulation */
		while(1) {
			char reason;
			struct tile_info *tile_info;
			if (guard_step(&guard, &reason) != true) {
				/* if guard cant step forward, check the reason why */
				switch(reason) {
				case '@': guard_turn_right(&guard); /* fall through */
				case '#': 
				{
					guard_turn_right(&guard);
					tile_info = tile_info_alloc(guard.location);
					if (tile_info->direction == guard.facing) {
						/* guard is entering a loop path */
						loop_count += 1;
						goto reset_tile_map;
					}
					tile_info->direction = guard.facing;
					continue;
				}
				case 0: goto reset_tile_map;
				default:
				}
			}
			guard_mark_position(&guard);
			//print_tile_map();
			//printf("\n\n");
		}
reset_tile_map:
		memcpy(tile_map, tile_map_copy, tile_size);
		reset_hash_table();
		reset_guard(&guard);
	}

	printf("guard loop count: %d\n", loop_count);
	free(tile_map);
	return 0;
}

