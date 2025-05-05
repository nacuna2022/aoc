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

static struct move_cost *new_move_cost(unsigned long tile_id,
	int cost, enum aoc_direction facing)
{
	return NULL;
}


static void analyze_tile(unsigned long tile_id, int cost, 
	struct aoc_minheap **cost_heap,	struct aoc_lut *visited)
{
	return;
}

static int maze_shortest_path(struct aoc_mapcache *maze)
{
	struct aoc_bot *reindeer;
	struct aoc_minheap *cost_heap;
	unsigned long tile_id;
	assert(maze != NULL);

	
	reindeer = aoc_new_bot(aoc_direction_right);
	if (reindeer == NULL) {
		aoc_die(-1, "cannot create reindeer\n");
	}

	/* cost_heap maps cost to move_cost pointer */
	if ((cost_heap = aoc_new_minheap(sizeof(struct move_cost *))) == NULL) {
		aoc_die(-1, "cannot create min heap\n");
	}

	/* add our starting tile with cost = 0 to the minheap to 
	 * get things started */
	struct move_cost *move_cost;
	aoc_mapcache_find_marker(maze, 'S');
	aoc_mapcache_tile(maze, &tile_id);
	move_cost = new_move_cost(tile_id, 0, aoc_direction_right);
	assert(move_cost != NULL);
	aoc_minheap_insert(&cost_heap, move_cost->cost, &move_cost, sizeof(struct move_cost*));

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

