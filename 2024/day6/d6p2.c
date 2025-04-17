#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include <aoc/mapcache.h>
#include <aoc/bot.h>
#include <aoc/lut.h>

static void init_guard_starting_point(struct aoc_mapcache *lab,
	unsigned long guard_start_tile_id)
{
	aoc_mapcache_goto_tile(lab, guard_start_tile_id);
	assert(aoc_mapcache_tile(lab, NULL) == '^');
	return;
}

static int lab_peek_direction(struct aoc_mapcache *lab,
	enum aoc_direction dir)
{
	switch(dir) {
	case aoc_direction_up:
		return aoc_mapcache_peek_up(lab);
	case aoc_direction_right:
		return aoc_mapcache_peek_right(lab);
	case aoc_direction_down:
		return aoc_mapcache_peek_down(lab);
	case aoc_direction_left:
		return aoc_mapcache_peek_left(lab);
	default:
		break;
	}

	/* we never go here. */
	assert(0);
	return -1;
}

static int guard_peek_front(struct aoc_mapcache *lab, struct aoc_bot *guard)
{
	enum aoc_direction front;
	assert(lab != NULL);
	assert(guard != NULL);
	front = aoc_bot_get_front(guard);
	return lab_peek_direction(lab, front);
}

static void guard_turn_right(struct aoc_bot *guard)
{
	assert(guard != NULL);
	aoc_bot_turn_right(guard);
	return;
}

static void lab_walk_direction(struct aoc_mapcache *lab,
	enum aoc_direction dir)
{
	switch(dir) {
	case aoc_direction_up:
		aoc_mapcache_step_up(lab);
		break;
	case aoc_direction_right:
		aoc_mapcache_step_right(lab);
		break;
	case aoc_direction_down:
		aoc_mapcache_step_down(lab);
		break;
	case aoc_direction_left:
		aoc_mapcache_step_left(lab);
		break;
	default:
		break;
	}
	return;
}

static void guard_walk_forward(struct aoc_mapcache *lab,
	struct aoc_bot *guard)
{
	enum aoc_direction front;
	assert(lab != NULL);
	assert(guard != NULL);
	front = aoc_bot_get_front(guard);
	lab_walk_direction(lab, front);
	return;
}

struct tile_info {
	enum aoc_direction guard_last_direction;
};

static int simulate_guard_patrol(struct aoc_mapcache *lab,
	unsigned long guard_start_tile_id)
{
	struct aoc_bot *guard;
	struct aoc_lut *lab_lut;
	bool guard_has_stepped_out = false;

	guard = aoc_new_bot(aoc_direction_up);
	lab_lut = aoc_new_lut(12, sizeof(struct tile_info), NULL);

	/* initialize the map with the guard starting tile */
	init_guard_starting_point(lab, guard_start_tile_id);
	
	for (;;) {
		int tile;
		unsigned long tile_id;

		/* peek guard front so he can try to walk forward */
		tile = guard_peek_front(lab, guard);

		/* if guard can walk out from this tile. simulation is done. */
		if (tile == -1) {
			guard_has_stepped_out = true;
			break;
		}
		
		/* if tile is a blocker e.g. it is '#', guard needs to 
		 * turn right */
		if (tile == '#') {
			struct aoc_lut_node *node;
			struct tile_info *info;
			enum aoc_direction guard_direction;

			/* guard might need to turn two times */
			while(tile == '#') {
				guard_turn_right(guard);
				tile = guard_peek_front(lab, guard);
			}
			aoc_mapcache_tile(lab, &tile_id);
			node = aoc_lut_lookup(lab_lut, tile_id);
			guard_direction = aoc_bot_get_front(guard);
			if (node != NULL) {
				info = aoc_lut_node_data(node);
				
				/* guard is trapped! */
				if (info->guard_last_direction == guard_direction) {
					break;
				}
			} else {
				node = aoc_lut_add(lab_lut, tile_id);
				info = aoc_lut_node_data(node);
				info->guard_last_direction = guard_direction;
			}
		}
		
		guard_walk_forward(lab, guard);
	}

	aoc_free_lut(lab_lut);
	aoc_free_bot(guard);
	return guard_has_stepped_out == true ? 0 : -1;
}

int main(void)
{
	struct aoc_mapcache *lab;
	unsigned long guard_start_tile_id = 0;
	int guard_trapped_count = 0;

	if ((lab = aoc_new_mapcache("input")) == NULL) {
		fprintf(stderr, "cannot open input file\n");
		return -1;
	}

	/* record the guard starting position */
	for (;;) {
		int tile;
		unsigned long tile_id;
		tile = aoc_mapcache_tile(lab, &tile_id);
		if (tile == '^') {
			guard_start_tile_id = tile_id;
			break;
		}

		if (aoc_mapcache_walk_forward(lab) == -1)
			break;
	}
	assert(guard_start_tile_id != 0);

	aoc_mapcache_reset(lab);
	for (;;) {
		int tile;
		unsigned long tile_id;

		/* add blocker to tile if we can */
		tile = aoc_mapcache_tile(lab, &tile_id);
		if (tile == '.')
			aoc_mapcache_change_tile(lab, '#');

		/* now we can start simulation */
		if (simulate_guard_patrol(lab, guard_start_tile_id) == -1)
			guard_trapped_count += 1;

		/* simulation might have ruined our lab position, 
		 * restore it first to where we were before simulation */
		aoc_mapcache_goto_tile(lab, tile_id);

		/* if we added a blocker to this tile, remove it */
		if (tile == '.')
			aoc_mapcache_change_tile(lab, '.');

		if (aoc_mapcache_walk_forward(lab) == -1)
			break;

	}

	printf("distinct blocker positions: %d\n", guard_trapped_count);
	aoc_free_mapcache(lab);
	return 0;
}

