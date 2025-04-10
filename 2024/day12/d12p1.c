#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>

#include <aoc/mapcache.h>
#include <aoc/lut.h>

static void add_garden_plot(struct aoc_lut *plot_lut, unsigned long id) 
{
	struct aoc_lut_node *lut_node;
	assert(plot_lut != NULL);
	lut_node = aoc_lut_add(plot_lut, id);
	assert(lut_node != NULL);
	return;
}

/* remember to remove in plot lut */
static void process_region(struct aoc_mapcache *garden, int region,
	struct aoc_lut *plot_lut, int *area, int *perimeter)
{
	unsigned long plot_id;
	int plant;
	int adj_plant;
	struct aoc_lut_node *lut_node;

	/* stop processing we have visited this plot before */
	plant = aoc_mapcache_tile(garden, &plot_id);
	lut_node = aoc_lut_lookup(plot_lut, plot_id);
	if (lut_node == NULL) {
		return;
	}

	/* are we crossing over to a new region of plants? */
	if (plant != region) {
		return;
	}

	/* if we reach here, we have not visited this plot before and we 
	 * are still in the same region. */
	aoc_lut_remove(lut_node);
	*area += 1;

	/* try go up from here */
	adj_plant = aoc_mapcache_peek_up(garden);
	if ((adj_plant != -1) && (adj_plant == region)) {
		aoc_mapcache_step_up(garden);
		process_region(garden, region, plot_lut, area, perimeter);
		aoc_mapcache_step_down(garden);
	} else {
		*perimeter += 1;
	}

	/* try go right from here */
	adj_plant = aoc_mapcache_peek_right(garden);
	if ((adj_plant != -1) && (adj_plant == region)) {
		aoc_mapcache_step_right(garden);
		process_region(garden, region, plot_lut, area, perimeter);
		aoc_mapcache_step_left(garden);
	} else {
		*perimeter += 1;
	}

	/* try go down from here */
	adj_plant = aoc_mapcache_peek_down(garden);
	if ((adj_plant != -1) && (adj_plant == region)) {
		aoc_mapcache_step_down(garden);
		process_region(garden, region, plot_lut, area, perimeter);
		aoc_mapcache_step_up(garden);
	} else {
		*perimeter += 1;
	}

	/* try go left from here */
	adj_plant = aoc_mapcache_peek_left(garden);
	if ((adj_plant != -1) && (adj_plant == region)) {
		aoc_mapcache_step_left(garden);
		process_region(garden, region, plot_lut, area, perimeter);
		aoc_mapcache_step_right(garden);
	} else {
		*perimeter += 1;
	}

	return;
}

int main(void)
{
	struct aoc_mapcache *garden;
	struct aoc_lut *plot_lut;
	int total_price = 0;

	garden = aoc_new_mapcache("input");
	plot_lut = aoc_new_lut(12, 0, NULL);
	assert(garden);
	assert(plot_lut);

	/* pass 1: attach metadata to all plots on the map */
	aoc_mapcache_reset(garden);
	for (;;) {
		unsigned long plot_id;
		int plant;
		plant = aoc_mapcache_tile(garden, &plot_id);
		assert(plant != -1);
		add_garden_plot(plot_lut, plot_id);
		if (aoc_mapcache_walk_forward(garden) == -1)
			break;
	}

	/* pass 2: for each unvisited tile we encounter... process the region
	 * where it belongs */
	aoc_mapcache_reset(garden);
	for (;;) {
		int plant;
		int area = 0;
		int perimeter = 0;
		plant = aoc_mapcache_tile(garden, NULL);
		process_region(garden, plant, plot_lut, &area, &perimeter);
		if (perimeter > 0) {
			assert(area > 0);
			total_price = total_price + (area * perimeter);
		}
		if (aoc_mapcache_walk_forward(garden) == -1)
			break;
	}
	printf("total price: %d\n", total_price);

	aoc_free_lut(plot_lut);
	aoc_free_mapcache(garden);
        return 0;
}

