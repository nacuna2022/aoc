#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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
		aoc_mapcache_change_tile(warehouse, row[i]);
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
	tmp_map = aoc_new_mapcache_grid(row, col, 0);
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

/* calling this guarantees there is a box infront of robot */
static int push_box(struct aoc_mapcache *warehouse,
	enum aoc_direction front)
{
	int ret = 0;
	assert(warehouse != NULL);
	
	assert(aoc_mapcache_peek_dir(warehouse, front) == 'O');

	/* walk foward to the blocker tile */
	aoc_mapcache_step_dir(warehouse, front);
	if (aoc_mapcache_peek_dir(warehouse, front) == 'O') {
		ret = push_box(warehouse, front);
	} else if (aoc_mapcache_peek_dir(warehouse, front) == '#') {
		/* if blocker is against a wall, we can't push it.
		 * do nothing. */
		ret = -1;
	}

	if (ret == 0) {
		/* we can push the tile */
		assert(aoc_mapcache_peek_dir(warehouse, front) == '.');
		assert(aoc_mapcache_tile(warehouse, NULL) == 'O');

		/* simulate box "pushing" by going to the front tile,
		 * changing it to a 'O', then going back to our tile, and 
		 * change it to '.' */
		aoc_mapcache_step_dir(warehouse, front);
		aoc_mapcache_change_tile(warehouse, 'O');
		step_back(warehouse, front);
		aoc_mapcache_change_tile(warehouse, '.');
	}

	/* step back original to our original tile prior 
	 * to calling push_box() */
	step_back(warehouse, front);
	return ret;
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
		} else if (tile == 'O') {
			/* robot needs to push box(es) to move */
			if (push_box(warehouse, front) == 0) {
				/* now confirm robot has pushed boxes and move.
				 * before, tile is 'O' and after moving it 
				 * should be '.' */
				assert(aoc_mapcache_peek_dir(warehouse, front) == '.');
				aoc_mapcache_step_dir(warehouse, front);
			} else {
				/* robot push blocker(s) that are against 
				 * a wall so it never pushed them. front should
				 * still be a blocker 'O'. so we cannot step */
				assert(aoc_mapcache_peek_dir(warehouse, front) == 'O');
			}

			//aoc_mapcache_show(warehouse);
		} else if (tile == '#'){
			/* nothing robot is blocked */
		}
	}
	return;
}

static int warehouse_sum_gps_coordinates(struct aoc_mapcache *warehouse)
{
	int sum_gps = 0;
	assert(warehouse != NULL);
	
	aoc_mapcache_absolute_reset(warehouse);
	for (;;) {
		if (aoc_mapcache_tile(warehouse, NULL) == 'O') {
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

