#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include <aoc/mapcache.h>
#include <aoc/lncache.h>
#include <aoc/queue.h>
#include <aoc/bot.h>
#include <aoc/die.h>

static enum aoc_direction symbol_to_command(int sym)
{
	enum aoc_direction cmd;
	switch(sym & 0xFF) {
	case '^' : cmd = aoc_direction_up; break;
	case '>' : cmd = aoc_direction_right; break;
	case 'v' : cmd = aoc_direction_down; break;
	case '<' : cmd = aoc_direction_left; break; 
	default: 
		assert(0);
	}
	return cmd;
}

static void fill_row(struct aoc_mapcache *warehouse,
	char *row, size_t row_len)
{
	size_t i;
	for (i = 0; i < row_len; i += 1) {
		switch(i[row]) {
		case '#': /* # becomes ## */
		case '.': /* . becomes .. */
			aoc_mapcache_change_tile(warehouse, i[row]);
			aoc_mapcache_walk_forward(warehouse);
			aoc_mapcache_change_tile(warehouse, i[row]);
			break;
		case 'O': /* O becomes [] */
			aoc_mapcache_change_tile(warehouse, '[');
			aoc_mapcache_walk_forward(warehouse);
			aoc_mapcache_change_tile(warehouse, ']');
			break;
		case '@': /* @ becomes @. */
			aoc_mapcache_change_tile(warehouse, '@');
			aoc_mapcache_walk_forward(warehouse);
			aoc_mapcache_change_tile(warehouse, '.');
			break;
		default:
			assert(0);
			break;
		}
		aoc_mapcache_walk_forward(warehouse);
	}
	return;
}

static int extract_warehouse_map(struct aoc_lncache *lncache, 
	struct aoc_mapcache **warehouse)
{
	size_t row;
	size_t col = 0;
	size_t i;
	assert(lncache != NULL);
	assert(warehouse != NULL);
	struct aoc_mapcache *tmp_map = NULL;
	struct aoc_line *line;
	size_t linelen; 
	char buffer[2048];
	
	/* first pass, get the map dimensions */
	for (row = 0; ; row +=1) {
		if (aoc_lncache_getline(lncache, &line, row) == -1) {
			/* this should not happen as the 
			 * warehouse map comes first before the 
			 * move list */
			assert(0);
		}

		aoc_line_get(line, buffer, sizeof buffer);
		
		/* if the first buffer is not a wall, e.g. not '#'
		 * we are not looking at the warehouse map anymore */
		if (buffer[0] != '#')
			break;

		linelen = aoc_line_strlen(line);
		assert(linelen < sizeof buffer);
		if (col == 0) {
			col = linelen - 1;
		}
		assert(col == (linelen -1)); /* map is a rigid square or rectangle */
	}

	assert(col > 0);
	tmp_map = aoc_new_mapcache_grid(row, col * 2, 0);
	if (tmp_map == NULL) {
		return -1;
	}

	/* second pass, we now fill up the map with our tiles */
	for (i = 0; i < row; i += 1) {
		aoc_lncache_getline(lncache, &line, i);
		aoc_line_get(line, buffer, sizeof buffer);
		fill_row(tmp_map, buffer, col);
	}
	aoc_mapcache_reset(tmp_map);
	*warehouse = tmp_map;
	return 0;
}

static int process_row(struct aoc_queue *command_queue,
	char *buffer, size_t count)
{
	int err;
	size_t i;
	assert(command_queue != NULL);
	assert(buffer != NULL);
	for (i = 0; i < count; i += 1) {
		enum aoc_direction dir;
		dir = symbol_to_command(i[buffer]);
		err = aoc_queue(command_queue, &dir, sizeof(enum aoc_direction));
		if (err == -1)
			return -1;
	}
	return 0;
}

static int extract_command_queue(struct aoc_lncache *lncache,
	struct aoc_queue **command_queue)
{
	char buffer[1024];
	struct aoc_line *line;
	size_t i;
	size_t line_len;

	assert(lncache != NULL);

	/* first pass, go to our move list */
	for (i =0; ; i += 1) {
		if (aoc_lncache_getline(lncache, &line, i) == -1) {
			/* this should not happen as the 
			 * warehouse map comes first before the 
			 * move list */
			assert(0);
		}
		aoc_line_get(line, buffer, sizeof buffer);
		line_len = aoc_line_strlen(line);
		assert(line_len < sizeof buffer);
		
		/* if the first buffer is not a wall, e.g. not '#'
		 * we are looking at the move list now. */
		if (buffer[0] != '#')
			break;
	}

	/* second pass, is our move list. turn it into a command queue */
	struct aoc_queue *tmp_queue = NULL;
	for (;; i +=1) {
		if (aoc_lncache_getline(lncache, &line, i) == -1) {
			break;
		}
		aoc_line_get(line, buffer, sizeof buffer);
		line_len = aoc_line_strlen(line);
		assert(line_len < sizeof buffer);
	
		if (tmp_queue == NULL) {
			tmp_queue = aoc_new_queue(sizeof(enum aoc_direction));
			assert(tmp_queue != NULL);
		}
		process_row(tmp_queue, buffer, line_len - 1);
	}
	assert(command_queue != NULL);
	*command_queue = tmp_queue;
	return 0;
}

/* assure robot is inside the box that needs to be verified */
static void assert_box(struct aoc_mapcache *warehouse)
{
	assert(warehouse != NULL);
	if (aoc_mapcache_tile(warehouse, NULL) == '[') {
		assert(aoc_mapcache_peek_dir(warehouse, aoc_direction_right) == ']');
		return;
	}
	assert(aoc_mapcache_tile(warehouse, NULL) == ']');
	assert(aoc_mapcache_peek_dir(warehouse, aoc_direction_left) == '[');
	return;
}

static enum aoc_direction get_opposite_direction(enum aoc_direction front)
{
	switch(front) {
	case aoc_direction_up: return aoc_direction_down;
	case aoc_direction_right: return aoc_direction_left;
	case aoc_direction_down: return aoc_direction_up;
	case aoc_direction_left: return aoc_direction_right;
	default:
		break;
	}
	/* will never get here */
	assert(0);
	return aoc_direction_limit;
}

static int step_back(struct aoc_mapcache *warehouse,
	enum aoc_direction front)
{
	enum aoc_direction back = get_opposite_direction(front);
	return aoc_mapcache_step_dir(warehouse, back);
}

static int push_box_horizontal(struct aoc_mapcache *warehouse,
	enum aoc_direction front)
{
	int ret = 0;
	int tile;
	int side1;
	int side2;

	assert(front == aoc_direction_left || front == aoc_direction_right);
	assert_box(warehouse);

	side1 = aoc_mapcache_tile(warehouse, NULL);
	side2 = side1 ^ 0b0110;

	/* step to the far side edge of the box */
	aoc_mapcache_step_dir(warehouse, front);
	assert(aoc_mapcache_tile(warehouse, NULL) == side2);

	/* then check the one after it is still a box */
	tile = aoc_mapcache_peek_dir(warehouse, front);
	if (tile == '[' || tile == ']') {
		assert(tile == side1);
		aoc_mapcache_step_dir(warehouse, front);
		ret = push_box_horizontal(warehouse, front);
		step_back(warehouse, front);
	} else if (tile == '#') { /* hard stop */
		ret = -1;
	}

	if (ret == 0) {
		aoc_mapcache_change_tile(warehouse, side1);
		aoc_mapcache_step_dir(warehouse, front);
		aoc_mapcache_change_tile(warehouse, side2);
		step_back(warehouse, front);
	}

	/* return to our initial side of the box */
	step_back(warehouse, front);
	if (ret == 0) {
		aoc_mapcache_change_tile(warehouse, '.');
	}
	//aoc_mapcache_show(warehouse);
	return ret;
}

/* ok so we have a near side and far side. near side is where we are at. 
 * far side is the opposite. */
static bool can_push_vertical(struct aoc_mapcache *warehouse,
	enum aoc_direction front, bool is_far_side)
{
	bool can_push = true;
	int near_side;
	enum aoc_direction side_dir;

	assert(warehouse != NULL);
	assert(front == aoc_direction_up || front == aoc_direction_down);

	/* process near side */
	near_side = aoc_mapcache_peek_dir(warehouse, front);
	if (near_side == '[' || near_side == ']') {
		aoc_mapcache_step_dir(warehouse, front);
		can_push = can_push_vertical(warehouse, front, false);
		step_back(warehouse, front);
	} else if (near_side == '.') {
		can_push = true;
	} else {
		return false;
	}

	if (is_far_side == false && can_push == true) {
		near_side = aoc_mapcache_tile(warehouse, NULL);
		side_dir = aoc_direction_left;
		if (near_side == '[')
			side_dir = aoc_direction_right;

		/* process far side */
		aoc_mapcache_step_dir(warehouse, side_dir);
		can_push = can_push_vertical(warehouse, front, true);
		step_back(warehouse, side_dir);
	}
	return can_push;
#if 0
	int near_side;
	int tile;
	bool can_push_near_side = true;
	bool can_push_far_side = true;

	assert(warehouse != NULL);
	assert(front == aoc_direction_up || front == aoc_direction_down);
	assert_box(warehouse);

	near_side = aoc_mapcache_tile(warehouse, NULL);

	/* process near side */
	tile = aoc_mapcache_peek_dir(warehouse, front);
	if (tile == '[' ||  tile == ']') {
		aoc_mapcache_step_dir(warehouse, front);
		can_push_near_side = can_push_vertical(warehouse, front, false);
		step_back(warehouse, front);
	} else if (tile == '#') {
		can_push_near_side = false;
	}

	if (can_push_near_side == false) {
		return false;
	}

	/* process far side */
	if (is_far_side == false) {
		enum aoc_direction side_dir;
		if (near_side == ']') {
			side_dir = aoc_direction_left;
		} else {
			side_dir = aoc_direction_right;
		}

		aoc_mapcache_step_dir(warehouse, side_dir);
		can_push_far_side = can_push_vertical(warehouse, front, true);
		step_back(warehouse, side_dir);
	}

	return can_push_near_side && can_push_far_side;
#endif
}

static void move_one_box(struct aoc_mapcache *warehouse, 
	enum aoc_direction front, bool is_far_side)
{
	int tile;
	assert(front == aoc_direction_up || front == aoc_direction_down);

	tile = aoc_mapcache_tile(warehouse, NULL);
	aoc_mapcache_step_dir(warehouse, front);
	aoc_mapcache_change_tile(warehouse, tile);
	step_back(warehouse, front);
	aoc_mapcache_change_tile(warehouse, '.');

	if (!is_far_side) {
		enum aoc_direction side_dir = aoc_direction_left;
		if (tile == '[')
			side_dir = aoc_direction_right;
		aoc_mapcache_step_dir(warehouse, side_dir);
		move_one_box(warehouse, front, true);
		step_back(warehouse, side_dir);
	}

	return;
}

/* only call this if we are 100% we can push boxes vertical. doesn't 
 * do any checks whatsoever */
static void actually_push_box_vertical(struct aoc_mapcache *warehouse,
	enum aoc_direction front)
{
	int near_side;
	int far_side;
	enum aoc_direction side_dir;

	assert(warehouse != NULL);
	assert_box(warehouse);
	assert(front == aoc_direction_up || front == aoc_direction_down);

	/* move up until the last box per side (near side) */
	near_side = aoc_mapcache_peek_dir(warehouse, front);
	if (near_side == '[' || near_side == ']') {
		aoc_mapcache_step_dir(warehouse, front);
		actually_push_box_vertical(warehouse, front);
		step_back(warehouse, front);
	}
	near_side = aoc_mapcache_tile(warehouse, NULL);
	assert(near_side == '[' || near_side == ']');

	/* now lets do the far side */
	side_dir = aoc_direction_left;
	if (near_side == '[')
		side_dir = aoc_direction_right;

	aoc_mapcache_step_dir(warehouse, side_dir);
	far_side = aoc_mapcache_peek_dir(warehouse, front);
	if (far_side == '[' || far_side == ']') {
		aoc_mapcache_step_dir(warehouse, front);
		actually_push_box_vertical(warehouse, front);
		step_back(warehouse, front);
	}
	far_side = aoc_mapcache_tile(warehouse, NULL);
	assert(near_side == (far_side ^ 0b0110));
	
	/* go back to our box's near side */
	step_back(warehouse, side_dir);
	move_one_box(warehouse, front, false);	
	return;
}

/* deal with both near and far side */
static int push_box_vertical(struct aoc_mapcache *warehouse,
	enum aoc_direction front)
{
	assert(warehouse != NULL);
	assert(front == aoc_direction_up || front == aoc_direction_down);
	if (can_push_vertical(warehouse, front, false) == true) {
		actually_push_box_vertical(warehouse, front);
		return 0;
	}
	return -1;
}

static int push_box(struct aoc_mapcache *warehouse,
	enum aoc_direction front)
{
	assert(warehouse != NULL);

	/* make sure we are inside a box */
	assert_box(warehouse);

	if (front == aoc_direction_right || front == aoc_direction_left) {
		return push_box_horizontal(warehouse, front);
	}

	assert((front == aoc_direction_up) || (front == aoc_direction_down));
	return push_box_vertical(warehouse, front);
}

static void simulate_robot(struct aoc_mapcache *warehouse,
	struct aoc_queue *command_queue)
{
	enum aoc_direction front;

	assert(warehouse != NULL);
	assert(command_queue != NULL);
	
	while(aoc_dequeue(command_queue, &front, sizeof front) != -1) {
		int tile;
		tile = aoc_mapcache_peek_dir(warehouse, front);
	
		/* bot will never be able to walk out because entire warehouse 
		 * area is enclosed with walls */
		assert(tile != -1);

		if (tile == '.') {
			/* robot can walk unimpeded */
			aoc_mapcache_step_dir(warehouse, front);
		} else if (tile == '[' || tile == ']') {
			aoc_mapcache_step_dir(warehouse, front);

			/* robot needs to push box(es) to move */
			if (push_box(warehouse, front) == 0) {
				/* now confirm robot has pushed boxes. robot
				 * should have been standing on the box tile
				 * but now it should be a '.' */
				assert(aoc_mapcache_tile(warehouse, NULL) == '.');
			} else {
				/* robot tried to push box(es) that are against 
				 * a wall so it never pushed them. front should
				 * still be a box. so we cannot step */
				assert_box(warehouse);

				/* undo our first step dir */
				step_back(warehouse, front);
			}

		} else if (tile == '#'){
			/* nothing robot is blocked */
		}

		//aoc_mapcache_show(warehouse);
	}
	return;
}

static int warehouse_sum_gps_coordinates(struct aoc_mapcache *warehouse)
{
	int sum_gps = 0;
	assert(warehouse != NULL);
	
	aoc_mapcache_absolute_reset(warehouse);
	for (;;) {
		if (aoc_mapcache_tile(warehouse, NULL) == '[') {
			int row;
			int col;
			int distance;

			aoc_mapcache_coord(warehouse, &row, &col);
			distance = (100 * row) + col;
			sum_gps += distance;
		}
		if (aoc_mapcache_walk_forward(warehouse) == -1) {
			break;
		}
	}
	return sum_gps;
}

int main()
{
	/* the file is a combination of both a mapcache and a lncache 
	 * separated by a single newline. so we open it first as a lncache 
	 * and create mapcache and our queue from there */
	struct aoc_lncache *lncache;
	struct aoc_mapcache *warehouse;
	struct aoc_queue *command_queue = NULL;
	int sum_gps;
	if ((lncache = aoc_new_lncache("input")) == NULL) {
		aoc_die(-1, "cannot open input file\n");
	}
	if (extract_warehouse_map(lncache, &warehouse) == -1) {
		aoc_die(-1, "extracting warehouse failed\n");
	}
	if (extract_command_queue(lncache, &command_queue) == -1) {
		aoc_die(-1, "extracting command queue failed\n");
	}
	aoc_free_lncache(lncache);
	//aoc_mapcache_show(warehouse);

	/* record starting tile and update it to a normal tile */
	if (aoc_mapcache_find_marker(warehouse, '@') == -1) {
		assert(0);
	}
	aoc_mapcache_set_start(warehouse);
	aoc_mapcache_change_tile(warehouse, '.');

	/* now we have a warehouse and the queue of commands (our movelist) */
	simulate_robot(warehouse, command_queue);

	/* robot has finished moving boxes around, now compute all boxes GPS */
	sum_gps = warehouse_sum_gps_coordinates(warehouse);
	printf("Sum GPS coordinates is %d\n", sum_gps);

	aoc_free_mapcache(warehouse);
	aoc_free_queue(command_queue);
	return 0;
}

