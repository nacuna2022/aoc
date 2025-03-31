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

#define HASH_TABLE_SHIFT	7
#define HASH_TABLE_SIZE		(1 << HASH_TABLE_SHIFT)
static struct trail *trail_hash_table[HASH_TABLE_SIZE] = {0};

static unsigned long fnv1a_hash(char *data, size_t size)
{
	size_t i;
	unsigned char *pdata = (unsigned char *)data;
	static const unsigned long fnv1a_offset = 0xcbf29ce484222325;
	static const unsigned long fnv1a_prime = 0x100000001b3;
	unsigned long h;
	h = fnv1a_offset;
	for (i = 0; i < size; i++) {
		h = h ^ pdata[i];
		h = h * fnv1a_prime;
	}
	return h;
}

static struct trail *new_trail(unsigned long unique_id9, 
	unsigned long unique_id0)
{
	struct trail *trail;
	trail = malloc(sizeof * trail);
	assert(trail != NULL);
	trail->trail_id = unique_id9;
	trail->trail_start = unique_id0;
	trail->link = NULL;
	return trail;
}

static int process_this_trail(unsigned long unique_id9,
	unsigned long unique_id0)
{
	int i;
	struct trail **trailpp;
	i = fnv1a_hash((char *)&unique_id9, sizeof unique_id9);
	i &= (HASH_TABLE_SIZE-1);
	trailpp = &trail_hash_table[i];
	for(;;) {
		struct trail *trailp = *trailpp;
		if (trailp == NULL)
			break;
		if (trailp->trail_id == unique_id9 && 
			trailp->trail_start == unique_id0) {
			return 0;
		}

		trailpp = &(trailp->link);
	}
	*trailpp = new_trail(unique_id9, unique_id0);
	return 1;
}

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
		return process_this_trail(unique_id9, unique_id0);
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

static void free_hash_chain(struct trail *trail)
{
	struct trail *ptr = trail;
	while(ptr != NULL) {
		struct trail *tmp = ptr->link;
		free(ptr);
		ptr = tmp;
	}
	return;
}

static void free_hash_table(struct trail **hash_tab,
		size_t size)
{
	size_t i;
	for(i = 0; i < size; i += 1) {
		struct trail *trail = hash_tab[i];
		if (trail == NULL)
			continue;
		free_hash_chain(trail);
		hash_tab[i] = NULL;
	}
	return;
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
	free_hash_table(trail_hash_table, HASH_TABLE_SIZE);
	return 0;
}

