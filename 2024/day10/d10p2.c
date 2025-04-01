#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <aoc/mapcache.h>

#define trailhead	'0'

struct trail {
	unsigned long trail_id; /* unique_id9 is used as our key */
	unsigned long trail_start;
	struct trail *link;
};

static int process_trailhead(struct aoc_mapcache *mapcache, int height,
	unsigned long unique_id0)
{
	int chnum;
	int score = 0;
	unsigned long unique_id9;
	chnum = aoc_mapcache_tile(mapcache, &unique_id9) - '0';
	if (chnum != height) {
		return 0;
	}

	if (height == 9) {
		assert(chnum == height);
		return 1;
	}

	height += 1;

	/* if we can go up from here, do so. */
	if (aoc_mapcache_step_up(mapcache) != -1) {
		score += process_trailhead(mapcache, height, unique_id0);
		aoc_mapcache_step_down(mapcache);
	}

	assert(aoc_mapcache_tile(mapcache, NULL)-'0' == chnum);

	/* if we can go right from here, do so. */
	if (aoc_mapcache_step_right(mapcache) != -1) {
		score += process_trailhead(mapcache, height, unique_id0);
		aoc_mapcache_step_left(mapcache);
	}
	
	assert(aoc_mapcache_tile(mapcache, NULL)-'0' == chnum);

	/* if we can go down from here, do so. */
	if (aoc_mapcache_step_down(mapcache) != -1) {
		score += process_trailhead(mapcache, height, unique_id0);
		aoc_mapcache_step_up(mapcache);
	}

	assert(aoc_mapcache_tile(mapcache, NULL)-'0' == chnum);

	/* if we can go left from here, do so. */
	if (aoc_mapcache_step_left(mapcache) != -1) {
		score += process_trailhead(mapcache, height, unique_id0);
		aoc_mapcache_step_right(mapcache);
	}
	
	assert(aoc_mapcache_tile(mapcache, NULL)-'0' == chnum);
	
	return score;
}

int main(void)
{
	struct aoc_mapcache *mapcache;
	int ch;
	int score = 0;

	mapcache = aoc_new_mapcache("input");
	if (mapcache == NULL) {
		return -1;
	}
	for (;;) {
		unsigned long unique_id;
		ch = aoc_mapcache_tile(mapcache, &unique_id);
		if (ch == trailhead) {
			score += process_trailhead(mapcache, 0, unique_id);	
		}
		if (aoc_mapcache_walk_forward(mapcache) == -1)
			break;
	}
	printf("score is %d\n", score);
	aoc_free_mapcache(mapcache);
	return 0;
}

