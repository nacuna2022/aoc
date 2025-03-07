#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>

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
} guard = {0};

static char *tile_map = NULL;
static int tile_size = 0;
static int tile_line_size = 0;

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
		if (new_ptr >= tile_map) {
			return new_ptr;
		}
		break;
	}
	default:
		assert(0);
	}
	return NULL;
}

static bool guard_mark_position(struct guard_struct *guard)
{
	char *tile_ptr = guard->location;
	if (*tile_ptr != 'X') {
		*tile_ptr = 'X';
		return true;
	}
	return false;
}

/* peek to see if there is a blocker in path */
static char guard_peek(struct guard_struct *guard)
{
	char *tile_ptr;
	tile_ptr = get_tile_from_direction(guard->location, guard->facing);
	if (tile_ptr) {
		return *tile_ptr;
	}
	return 0;
}

/* we step on the direction where guard is facing currently */
static bool guard_step(struct guard_struct *guard, char *reason)
{
	char ch;
	ch = guard_peek(guard);
	if ((ch != 0) && (ch != '#')) {
		guard->location = get_tile_from_direction(guard->location,
				guard->facing);
		return true;
	}
	*reason = ch;
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

#if 0
static void guard_turn_left(struct guard_struct *guard)
{
	guard_turn(guard, 3);
	return;
}
#endif

static void init_guard(struct guard_struct *guard, char *location,
		enum cardinal_direction facing)
{
	guard->location = location;
	guard->facing = facing;
	return;
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


int main(void)
{
	struct stat input_stat;
	int input_fd;
	int err;
	char ch;
	char *guard_loc_ptr = NULL;
	size_t newline_count = 0;
	int unique_positions = 0;
	
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

	/* make sure we have a guard facing the up direction */
	assert(guard_loc_ptr != NULL);
	tile_line_size = tile_size / newline_count;

	/* we count the initial position of the guard as part
	 * of the unique position */
	unique_positions += guard_mark_position(&guard) == true ? 1 : 0;
	while(1) {
		char reason;
		if (guard_step(&guard, &reason) != true) {
			/* if guard cant step forward, check the reason why */
			if (reason == '#') {
				guard_turn_right(&guard);
			} else if (reason == 0) {
				/* guard has stepped out of the tile map */
				break;
			}
			continue;
		}

		unique_positions += guard_mark_position(&guard) == true ? 1 : 0;
	}

	printf("unique positions count: %d\n", unique_positions);
	free(tile_map);
	return 0;
}

