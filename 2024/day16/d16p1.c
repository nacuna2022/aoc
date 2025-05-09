#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <aoc/mapcache.h>
#include <aoc/die.h>
#include <aoc/minheap.h>
#include <aoc/lut.h>

struct move_key {
	unsigned long tile_id;
	enum aoc_direction dir;
};

struct movement {
	int cost;
	struct move_key key;
	bool visited;
};

static enum aoc_direction get_opposite_dir(enum aoc_direction dir)
{
	if (dir == aoc_direction_up)
		return aoc_direction_down;
	if (dir == aoc_direction_right)
		return aoc_direction_left;
	if (dir == aoc_direction_down)
		return aoc_direction_up;
	assert(dir == aoc_direction_left);
	return aoc_direction_right;
}

static void step_back(struct aoc_mapcache *maze, enum aoc_direction dir)
{
	dir = get_opposite_dir(dir);
	aoc_mapcache_step_dir(maze, dir);
	return;
}

static struct movement *new_movement(struct aoc_lut *lut,
	int cost, unsigned long tile_id,
	enum aoc_direction dir)
{
	struct movement *move;
	if ((move = malloc(sizeof *move)) != NULL) {
		memset(move, 0, sizeof *move);
		move->cost = cost;
		move->key.dir = dir;
		move->key.tile_id = tile_id;
		move->visited = false;

		struct move_key move_key = {0};
		move_key.dir = dir;
		move_key.tile_id = tile_id;
		aoc_lut_add(lut, &move_key, sizeof move_key, &move, sizeof(struct movement *));
	}
	return move;
}

static void analyze_direction(struct aoc_mapcache *maze,
	enum aoc_direction dir, struct movement *prev_movement,
	struct aoc_minheap **heap, struct aoc_lut *lut)
{
	unsigned long tile_id;
	struct movement *movement;
	int new_cost = prev_movement->cost + 1;
	int err;
	struct move_key key = {0};

	/* step into the direction */
	aoc_mapcache_step_dir(maze, dir);
	if (aoc_mapcache_tile(maze, &tile_id) == '#') {
		goto exit;
	}

	if (dir != prev_movement->key.dir) {
		new_cost += 1000;
	}

	key.tile_id = tile_id;
	key.dir = dir;

	/* get the movement if it exists */
	err = aoc_lut_lookup(lut, &key, sizeof key, &movement, sizeof(struct movement *));
	if (err == - 1) {
		/* this is a new movement */
		movement = new_movement(lut, new_cost, tile_id, dir);
		aoc_minheap_insert(heap, movement->cost, &movement, sizeof(struct movement *));
	} else if (movement->visited == false) { 
		/* existing movement. we might need to update it with cheaper
		 * cost. let's check. */
		if (new_cost < movement->cost) {
			movement->cost = new_cost;

			/* add new cost to minheap ... there will be an old 
			 * "stale" version of this with bigger cost. this is okay
			 * since our newer version with cheaper cost will 
			 * take precendence over it. */
			aoc_minheap_insert(heap, movement->cost, &movement, sizeof(struct movement *));
		}
	}

exit:
	/* step back to where we were */
	step_back(maze, dir);
	return;
}

static void analyze_tile(struct aoc_mapcache *maze, struct movement *movement,
	struct aoc_minheap **heap, struct aoc_lut *lut)
{
	analyze_direction(maze, aoc_direction_up, movement, heap, lut);
	analyze_direction(maze, aoc_direction_right, movement, heap, lut);
	analyze_direction(maze, aoc_direction_down, movement, heap, lut);
	analyze_direction(maze, aoc_direction_left, movement, heap, lut);
	return;
}

static void free_movement(const void *key, const void *data, void *param)
{
	struct movement **move;
	(void)key;
	(void)param;
	move = (struct movement **)data;
	free(*move);
	return;
}

static int maze_find_shortest_path(struct aoc_mapcache *maze)
{
	int score = 0;
	struct aoc_lut *lut;
	struct movement *movement;
	struct aoc_minheap *heap;
	unsigned long tile_id;
	assert(maze != NULL);

	if ((lut = aoc_new_lut(12, sizeof(struct move_key), sizeof(struct movement *))) == NULL) {
		aoc_die(-1, "cannot allocate movement lookup table\n");
	}
	if ((heap = aoc_new_minheap(sizeof(struct movement *))) == NULL) {
		aoc_die(-1, "cannot allocate binary heap\n");
	}

	/* initially add the starter tile onto our heap */
	assert(aoc_mapcache_find_marker(maze, 'S') == 'S');
	assert(aoc_mapcache_tile(maze, &tile_id) == 'S');
	movement = new_movement(lut, 0, tile_id, aoc_direction_right);
	aoc_minheap_insert(&heap, movement->cost, &movement, sizeof(struct movement *));
	for (;;) {
		int cost;

		/* extract the tile with cheapest movement */
		aoc_minheap_get(heap, &cost, &movement, sizeof(struct movement *));

		/* is this the ending tile? */
		if ((aoc_mapcache_goto_tile(maze, movement->key.tile_id)) == 'E') {
			assert(cost == movement->cost);
			score = movement->cost;
			break;
		}

		analyze_tile(maze, movement, &heap, lut);
		movement->visited = true;
	}
	aoc_free_minheap(heap);
	aoc_lut_foreach(lut, free_movement, NULL);
	aoc_free_lut(lut);
	return score;
}

int main(void)
{
	struct aoc_mapcache *maze;
	int score = 0;
	if ((maze = aoc_new_mapcache("input")) == NULL) {
		aoc_die(-1, "cannot create map cache\n");
	}
	score = maze_find_shortest_path(maze);
	printf("shortest path score is: %d\n", score);
	aoc_free_mapcache(maze);
	return 0;
}

