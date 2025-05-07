#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#include <aoc/mapcache.h>
#include <aoc/die.h>
#include <aoc/bot.h>
#include <aoc/minheap.h>
#include <aoc/lut.h>

struct move {
	int cost;
	unsigned long tile_id;
	enum aoc_direction facing;
};

struct move_key {
	unsigned long tile_id;
	enum aoc_direction facing;
};

static enum aoc_direction get_opposite_direction(enum aoc_direction dir)
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
	dir = get_opposite_direction(dir);
	aoc_mapcache_step_dir(maze, dir);
	return;
}

static struct move *new_move(int cost, enum aoc_direction facing,
	unsigned long tile_id)
{
	struct move *move;
	move = malloc(sizeof *move);
	assert(move != NULL);
	move->cost = cost;
	move->facing = facing;
	move->tile_id = tile_id;
	return move;
}

static void analyze_one_tile(struct aoc_lut *lut, unsigned long tile_id,
	int new_cost, enum aoc_direction dir, struct aoc_minheap **heap)
{
	int err;
	struct move *move = NULL;
	bool update_minheap = false;
	struct move_key move_key = { .tile_id = tile_id, .facing = dir, };
	err = aoc_lut_lookup(lut, &move_key, sizeof move_key, &move, sizeof(struct move *));
	if (err == -1) {
		/* we are looking at a brand new tile */
		move = new_move(new_cost, dir, tile_id);
		aoc_lut_add(lut, &move_key, sizeof move_key, &move, sizeof(struct move *));
		update_minheap = true;
	} else if (new_cost < move->cost) {
		move->cost = new_cost;
		move->tile_id = tile_id;
		move->facing = dir;
		update_minheap = true;
	}

	if (update_minheap == true) {
		aoc_minheap_insert(heap, new_cost, &move, sizeof(struct move *));
	}
	
	return;
}

static void analyze_tile_dir(struct aoc_mapcache *maze, 
	struct aoc_bot *reindeer, enum aoc_direction dir, int prev_cost,
	struct aoc_lut *lut, struct aoc_minheap **heap)
{
	int tile;
	unsigned long tile_id;

	/* step into direction */
	aoc_mapcache_step_dir(maze, dir);
	tile = aoc_mapcache_tile(maze, &tile_id);
	assert(tile != -1);
	if (tile != '#') {
		int new_cost = prev_cost + 1;
		if (dir != aoc_bot_get_front(reindeer)) {
			new_cost += 1000;
		}
		analyze_one_tile(lut, tile_id, new_cost, dir, heap);
	}
	/* step back to our initial tile */
	step_back(maze, dir);
	return;
}

static void analyze_tile(struct aoc_mapcache *maze, struct aoc_minheap **heap,
	struct aoc_lut *lut, struct move *move, struct aoc_bot *reindeer)
{
	int row;
	int col;
	aoc_mapcache_coord(maze, &row, &col);
	analyze_tile_dir(maze, reindeer, aoc_direction_up, move->cost, lut, heap);
	analyze_tile_dir(maze, reindeer, aoc_direction_right, move->cost, lut, heap);
	analyze_tile_dir(maze, reindeer, aoc_direction_down, move->cost, lut, heap);
	analyze_tile_dir(maze, reindeer, aoc_direction_left, move->cost, lut, heap);
	return;
}

static void free_move(const void *key, const void *data, void *param)
{
	(void)key;
	(void)param;
	struct move **move = (struct move **)data;
	free(*move);
	return;
}

static int maze_shortest_path(struct aoc_mapcache *maze)
{
	struct aoc_bot *reindeer;
	struct aoc_minheap *heap;
	struct aoc_lut *lut;
	unsigned long tile_id;
	struct move *move;
	int tile;
	int cost = -1;

	assert(maze != NULL);

	reindeer = aoc_new_bot(aoc_direction_right);
	if (reindeer == NULL) {
		aoc_die(-1, "cannot create reindeer\n");
	}

	if ((heap = aoc_new_minheap(sizeof(struct move *))) == NULL) {
		aoc_die(-1, "cannot create min heap\n");
	}

	if ((lut = aoc_new_lut(12, sizeof(struct move_key), sizeof(struct move *))) == NULL) {
		aoc_die(-1, "cannot create lut\n");
	}

	assert(aoc_mapcache_find_marker(maze, 'S') == 'S');
	assert(aoc_mapcache_tile(maze, &tile_id) == 'S');
	move = new_move(0, aoc_direction_right, tile_id);
	aoc_minheap_insert(&heap, 0, &move, sizeof(struct move *));
	struct move_key move_key = {. tile_id = tile_id, .facing = aoc_direction_right };
	aoc_lut_add(lut, &move_key, sizeof move_key, &move, sizeof(struct move *));

	for (;;) {
		int cheapest_cost;

		/* extract the cheapest tile to move to */
		aoc_minheap_get(heap, &cheapest_cost, &move, sizeof(struct move*));

		/* if this is our end tile, journey is complete */
		aoc_mapcache_goto_tile(maze, move->tile_id);
		if ((tile = aoc_mapcache_tile(maze, NULL)) == 'E') {
			cost = cheapest_cost;
			break;
		}
		/* will never happen. reindeer can't walk to a wall */
		assert(tile != 'E');

		aoc_bot_init(reindeer, move->facing);

		/* now we can analyze where reindeer is at */
		analyze_tile(maze, &heap, lut, move, reindeer);
	}
	
	aoc_lut_foreach(lut, free_move, NULL);

	aoc_free_lut(lut);
	aoc_free_bot(reindeer);
	aoc_free_minheap(heap);
	return cost;
}

int main(void)
{
	struct aoc_mapcache *maze;
	int score;
	if ((maze = aoc_new_mapcache("input")) == NULL) {
		aoc_die(-1, "cannot open input file\n");
	}
	score = maze_shortest_path(maze);
	printf("score is %d\n", score);

	aoc_free_mapcache(maze);
	return 0;
}

