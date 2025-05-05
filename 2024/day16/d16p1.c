#include <stdio.h>
#include <assert.h>

#include <aoc/mapcache.h>
#include <aoc/die.h>
#include <aoc/bot.h>
#include <aoc/minheap.h>

struct move_info {
	unsigned long tile_id;
	enum aoc_direction facing;
	int cost;
};

static int maze_shortest_path(struct aoc_mapcache *maze)
{
	struct aoc_bot *reindeer;
	struct aoc_minheap *cost_heap;
	assert(maze != NULL);
	
	reindeer = aoc_new_bot(aoc_direction_right);
	if (reindeer == NULL) {
		aoc_die(-1, "cannot create reindeer\n");
	}

	cost_heap = aoc_new_minheap(sizeof(struct move_info));




	for (;;) {
		int cost;
		int tile;
		unsigned long tile_id;

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

