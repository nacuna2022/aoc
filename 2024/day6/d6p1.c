#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include <aoc/mapcache.h>
#include <aoc/bot.h>
#include <aoc/lut.h>

static void init_guard_starting_point(struct aoc_mapcache *lab)
{
	int tile;
	while((tile = aoc_mapcache_walk_forward(lab)) != -1) {
		if (tile == '^') {
			aoc_mapcache_set_start(lab);
			break;
		}
	}
	assert(tile == '^');
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

int main(void)
{
	struct aoc_mapcache *lab;
	struct aoc_bot *guard;
	struct aoc_lut *lab_lut;
	int distinct_positions = 0;

	lab = aoc_new_mapcache("input");
	guard = aoc_new_bot(aoc_direction_up);
	lab_lut = aoc_new_lut(12, 0, NULL);

	/* initialize the map with the guard starting tile */
	init_guard_starting_point(lab);

	for (;;) {
		int tile;
		unsigned long tile_id;

		/* check to see if guard has visited this tile prior */
		aoc_mapcache_tile(lab, &tile_id);
		if (aoc_lut_lookup(lab_lut, tile_id) == NULL) {
			aoc_lut_add(lab_lut, tile_id);
			distinct_positions += 1;
		}

		/* peek guard front so he can try to walk forward */
		tile = guard_peek_front(lab, guard);

		/* if guard can walk out from this tile. simulation is done. */
		if (tile == -1)
			break;
		
		/* if tile is a blocker e.g. it is '#', guard needs to 
		 * turn right */
		if (tile == '#') {
			guard_turn_right(guard);
			continue;
		}
		
		guard_walk_forward(lab, guard);
	}
	printf("distinct positions: %d\n", distinct_positions);

	aoc_free_lut(lab_lut);
	aoc_free_bot(guard);
	aoc_free_mapcache(lab);
	return 0;
}

