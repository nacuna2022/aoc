#include <stdio.h>
#include <assert.h>

#include <aoc/mapcache.h>
#include <aoc/die.h>
#include <aoc/bot.h>
#include <aoc/minheap.h>
#include <aoc/lut.h>

struct move_cost {
	unsigned long tile_id;
	int cost;
	enum aoc_direction facing;
};


static void analyze_tile(int cost, struct aoc_minheap **cost_heap,
	struct aoc_lut *visited)
{
	return;
}

static int maze_shortest_path(struct aoc_mapcache *maze)
{
	struct aoc_bot *reindeer;
	struct aoc_minheap *cost_heap;
	struct aoc_lut *unvisited;
	unsigned long tile_id;
	assert(maze != NULL);

	
	reindeer = aoc_new_bot(aoc_direction_right);
	if (reindeer == NULL) {
		aoc_die(-1, "cannot create reindeer\n");
	}

	/* cost_host maps cost to tile_id */
	if ((cost_heap = aoc_new_minheap(sizeof(unsigned long))) == NULL) {
		aoc_die(-1, "cannot create min heap\n");
	}

	/* lut maps tile_id to initial cost of unvisited node */
	if ((unvisited = aoc_new_lut(12, sizeof(unsigned long),  sizeof(struct move_cost))) == NULL) {
		aoc_die(-1, "cannot create cost lookup table\n");
	}

	/* add our starting tile with cost = 0 to the minheap to 
	 * get things started */
	aoc_mapcache_find_marker(maze, 'S');
	aoc_mapcache_tile(maze, &tile_id);
	aoc_minheap_insert(&cost_heap, 0, &tile_id, sizeof tile_id);

	struct move_cost start_cost = {
		.tile_id = tile_id,
		.cost = 0,
		.facing = aoc_direction_right,
	};
	aoc_lut_add(unvisited, &tile_id, sizeof tile_id, &start_cost, sizeof start_cost);

	for (;;) {
		int cost;
		int tile;

		/* extract the tile with cheapest move cost */
		aoc_minheap_get(cost_heap, &cost, &tile_id, sizeof tile_id);

		/* if this tile is the end tile, we are done. */
		aoc_mapcache_goto_tile(maze, tile_id);
		if ((tile = aoc_mapcache_tile(maze, NULL)) == 'E') {
			// save the cost. this is the score.
			break;
		}

		//analyze_tile();
	
	}

	aoc_free_bot(reindeer);
	return 0;
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

