#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <aoc/mapcache.h>
#include <aoc/lut.h>
#include <aoc/dlist.h>
#include <aoc/bot.h>

struct garden_region {
	int plant;
	int area;
	struct aoc_dlist_node node; /* next region */

	struct aoc_dlist_node top_edges;
	struct aoc_dlist_node bottom_edges;
	struct aoc_dlist_node right_edges;
	struct aoc_dlist_node left_edges;
};

struct plot_info {
	struct garden_region *region;
};

struct point {
	int x;
	int y;
};

struct edge {
	struct point start;
	struct point end;
	struct aoc_dlist_node node; /* link to region edges */
};

static struct garden_region *new_garden_region(int plant)
{
	struct garden_region *region;
	region = malloc(sizeof(struct garden_region));
	assert(region != NULL);
	region->plant = plant;
	region->area = 0;

	aoc_dlist_init(&region->top_edges);
	aoc_dlist_init(&region->bottom_edges);
	aoc_dlist_init(&region->right_edges);
	aoc_dlist_init(&region->left_edges);
	return region;
}

static void garden_plot_scan_region(struct aoc_mapcache *garden, 
	struct garden_region *region, struct aoc_lut *plot_lut)
{
	int err;
	unsigned long plot_id;
	int plant;
	int adj_plant;
	struct plot_info plot_info;

	/* stop processing. we have visited this plot before */
	plant = aoc_mapcache_tile(garden, &plot_id);
	err = aoc_lut_lookup(plot_lut, &plot_id, sizeof plot_id, &plot_info, sizeof plot_info);
	if (err == 0) {
		return;
	}

	/* are we crossing over to a new region of plants? */
	if (plant != region->plant) {
		return;
	}

	/* add this new area to this region */
	plot_info.region = region;
	region->area += 1;
	
	/* add this lut again with update information */
	err = aoc_lut_add(plot_lut, &plot_id, sizeof plot_id,
			&plot_info, sizeof plot_info);
	assert(err == 0);

	/* try to go up from here */
	adj_plant = aoc_mapcache_peek_up(garden);
	if ((adj_plant != -1) && (adj_plant == region->plant)) {
		aoc_mapcache_step_up(garden);
		garden_plot_scan_region(garden, region, plot_lut);
		aoc_mapcache_step_down(garden);
	}

	/* try to go right from here */
	adj_plant = aoc_mapcache_peek_right(garden);
	if ((adj_plant != -1) && (adj_plant == region->plant)) {
		aoc_mapcache_step_right(garden);
		garden_plot_scan_region(garden, region, plot_lut);
		aoc_mapcache_step_left(garden);
	}

	/* try to go down from here */
	adj_plant = aoc_mapcache_peek_down(garden);
	if ((adj_plant != -1) && (adj_plant == region->plant)) {
		aoc_mapcache_step_down(garden);
		garden_plot_scan_region(garden, region, plot_lut);
		aoc_mapcache_step_up(garden);
	}

	/* try to go left from here */
	adj_plant = aoc_mapcache_peek_left(garden);
	if ((adj_plant != -1) && (adj_plant == region->plant)) {
		aoc_mapcache_step_left(garden);
		garden_plot_scan_region(garden, region, plot_lut);
		aoc_mapcache_step_right(garden);
	}

	return;
}

static bool edges_are_the_same(struct edge *edge1, struct edge *edge2)
{
	assert(edge1 != NULL);
	assert(edge2 != NULL);
	static const size_t size = sizeof(struct point);
	if ((memcmp(&edge1->start, &edge2->start, size) == 0) &&
		(memcmp(&edge2->end, &edge2->end, size) == 0)) {
		return true;
	}
	return false;
}

static bool has_duplicate_edge(struct aoc_dlist_node *edge_list, 
	struct edge *edge)
{
	struct edge *tmp_edge;
	struct aoc_dlist_node *node;
	assert(edge_list != NULL);
	assert(edge != NULL);
	for (node = edge_list->next; node != edge_list; node = node->next) {
		tmp_edge = aoc_dlist_container(node, offsetof(struct edge, node));
		if (edges_are_the_same(edge, tmp_edge) == true)
			return true;
	}
	return false;
}

/* for horizontal checking */
static bool same_row_edges(struct edge *edge1, struct edge *edge2)
{
	if (edge1->start.x == edge2->start.x) {
		assert(edge1->end.x == edge2->end.x);
		return true;
	}
	return false;
}

/* assumption here is edge2 must come after edge1 */
static bool adjacent_row_edges(struct edge *edge1, struct edge *edge2)
{
	/* adjacent edges should be same rows as well */
	if (same_row_edges(edge1, edge2) == true) {
		if (edge1->end.y == edge2->start.y) {
			return true;
		}
	}
	return false;
}

/* combine edge2 to edge1 e.g. edge2 will be made to disappear soon */
static void combine_row_edges(struct edge *edge1, struct edge *edge2)
{
	edge1->end.y = edge2->end.y;
	return;
}

static bool coalesce_horizontal_edge(struct aoc_dlist_node *edge_list,
	struct edge *edge)
{
	struct edge *tmp_edge;
	struct aoc_dlist_node *node;
	assert(edge_list != NULL);
	assert(edge != NULL);

	/* coalesce back */
	for (node = edge_list->next; node != edge_list; node = node->next) {
		tmp_edge = aoc_dlist_container(node, offsetof(struct edge, node));
		if (adjacent_row_edges(tmp_edge, edge) == true) {
			combine_row_edges(tmp_edge, edge);
			return true;
		}
	}
	return false;
}

/* for vertical checking */
static bool same_col_edges(struct edge *edge1, struct edge *edge2)
{
	if (edge1->start.y == edge2->start.y) {
		assert(edge1->end.y == edge2->end.y);
		return true;
	}
	return false;
}

/* assumption here is edge2 must come after edge1 */
static bool adjacent_col_edges(struct edge *edge1, struct edge *edge2)
{
	/* adjacent edges should be same column as well */
	if (same_col_edges(edge1, edge2) == true) {
		if (edge1->end.x == edge2->start.x) {
			return true;
		}
	}
	return false;
}

/* combine edge2 to edge1 e.g. edge2 will be made to disappear soon */
static void combine_col_edges(struct edge *edge1, struct edge *edge2)
{
	edge1->end.x = edge2->end.x;
	return;
}

static bool coalesce_vertical_edge(struct aoc_dlist_node *edge_list,
	struct edge *edge)
{
	struct edge *tmp_edge;
	struct aoc_dlist_node *node;
	assert(edge_list != NULL);
	assert(edge != NULL);

	/* coalesce back */
	for (node = edge_list->next; node != edge_list; node = node->next) {
		tmp_edge = aoc_dlist_container(node, offsetof(struct edge, node));
		if (adjacent_col_edges(tmp_edge, edge) == true) {
			combine_col_edges(tmp_edge, edge);
			return true;
		}
	}
	return false;
}

static struct edge *new_edge(struct edge *edge)
{
	struct edge *tmp;
	if ((tmp = malloc(sizeof * tmp)) != NULL) {
		*tmp = *edge;
	}
	return tmp;
}

static void region_add_horizontal_edge(struct aoc_mapcache *garden,
	struct aoc_dlist_node *edge_list, struct edge *edge)
{
	assert(garden != NULL);
	assert(edge_list != NULL);
	assert(edge != NULL);

	/* first find duplicate. some edges are shared between two plots */
	if (has_duplicate_edge(edge_list, edge) == true) {
		return;
	}

	/* then try to coalesce. adjacent edges grow, otherwise, add new 
	 * unique edge */
	if (coalesce_horizontal_edge(edge_list, edge) == false) {
		struct edge *tmp = new_edge(edge);
		assert(tmp != NULL);
		aoc_dlist_prepend(edge_list, &tmp->node);
	}
	return;
}

static void region_add_vertical_edge(struct aoc_mapcache *garden,
	struct aoc_dlist_node *edge_list, struct edge *edge)
{
	assert(garden != NULL);
	assert(edge_list != NULL);
	assert(edge != NULL);

	/* first find duplicate. some edges are shared between two plots */
	if (has_duplicate_edge(edge_list, edge) == true) {
		return;
	}

	/* then try to coalesce. adjacent edges grow, otherwise, add new 
	 * unique edge */
	if (coalesce_vertical_edge(edge_list, edge) == false) {
		struct edge *tmp = new_edge(edge);
		assert(tmp != NULL);
		aoc_dlist_prepend(edge_list, &tmp->node);
	}

	return;
}

static bool edge_is_boundary(struct aoc_mapcache *garden, 
	enum aoc_direction dir, struct garden_region *region, int plant)
{
	switch (dir) {
	case aoc_direction_up:
		plant = aoc_mapcache_peek_up(garden);
		break;
	case aoc_direction_right:
		plant = aoc_mapcache_peek_right(garden);
		break;
	case aoc_direction_down:
		plant = aoc_mapcache_peek_down(garden);
		break;
	case aoc_direction_left:
		plant = aoc_mapcache_peek_left(garden);
		break;
	default:
		assert(false);
	}
	return plant != region->plant;
}

static void init_horizontal_edge(struct edge *edge, int x, int y)
{
	edge->start.x = x;
	edge->start.y = y;
	edge->end.x = x;
	edge->end.y = y + 1;
	return;
}

static void init_vertical_edge(struct edge *edge, int x, int y)
{
	edge->start.x = x;
	edge->start.y = y;
	edge->end.x = x + 1;
	edge->end.y = y;
	return;
}

static void get_edge_coord(struct aoc_mapcache *garden, struct edge *edge,
	enum aoc_direction dir)
{
	int x;
	int y;
	assert(garden != NULL);
	assert(edge != NULL);

	aoc_mapcache_coord(garden, &x, &y);
	switch(dir) {
	case aoc_direction_up:
		init_horizontal_edge(edge, x, y);
		break;
	case aoc_direction_right:
		init_vertical_edge(edge, x, y+1);
		break;
	case aoc_direction_down:
		init_horizontal_edge(edge, x+1, y);
		break;
	case aoc_direction_left:
		init_vertical_edge(edge, x, y);
		break;
	default: 
		assert(0);
	}
	return;
}

static void garden_plot_edges(struct aoc_mapcache *garden, 
	struct aoc_lut *plot_lut)
{
	int err;
	int plant;
	unsigned long plot_id;
	struct plot_info plot_info;
	struct garden_region *region;
	int x;
	int y;

	assert(garden != NULL);
	assert(plot_lut != NULL);

	/* get plot_info of current tile */
	plant = aoc_mapcache_tile(garden, &plot_id);
	err = aoc_lut_lookup(plot_lut, &plot_id, sizeof plot_id, &plot_info, sizeof plot_info);
	assert(err == 0);
	assert(plot_info.region != NULL);

	/* we now have a reference to the region where this tile belongs */
	assert(plot_info.region->plant == plant);
	region = plot_info.region;

	aoc_mapcache_coord(garden, &x, &y);
	if (edge_is_boundary(garden, aoc_direction_up, region, plant) == true) {
		struct edge top_edge;
		get_edge_coord(garden, &top_edge, aoc_direction_up);
		region_add_horizontal_edge(garden, &region->top_edges, &top_edge);
	}

	if (edge_is_boundary(garden, aoc_direction_right, region, plant) == true) {
		struct edge right_edge;
		get_edge_coord(garden, &right_edge, aoc_direction_right);
		region_add_vertical_edge(garden, &region->right_edges, &right_edge);
	}

	if (edge_is_boundary(garden, aoc_direction_down, region, plant) == true) {
		struct edge bottom_edge;
		get_edge_coord(garden, &bottom_edge, aoc_direction_down);
		region_add_horizontal_edge(garden, &region->bottom_edges, &bottom_edge);
	}

	if (edge_is_boundary(garden, aoc_direction_left, region, plant) == true) {
		struct edge left_edge;
		get_edge_coord(garden, &left_edge, aoc_direction_left);
		region_add_vertical_edge(garden, &region->left_edges, &left_edge);
	}

	return;
}

#if 0
static void print_edges(struct aoc_dlist_node *edge_list)
{
	struct aoc_dlist_node *node;
	struct edge *edge;
	assert(edge_list != NULL);
	for (node = edge_list->next; node != edge_list; node = node->next) {
		edge = aoc_dlist_container(node, offsetof(struct edge, node));
		printf("(%d:%d) -> (%d,%d)\n",
			edge->start.x, edge->start.y, edge->end.x, edge->end.y);
	}
	return;
}
#endif

static int count_nodes(struct aoc_dlist_node *edge_list)
{
	struct aoc_dlist_node *node;
	int count = 0;
	assert(edge_list != NULL);
	for (node = edge_list->next; node != edge_list; node = node->next) {
		count += 1;
	}
	return count;
}

static int region_get_price(struct garden_region *region)
{
	int v_edges;
	int h_edges;
	assert(region != NULL);

	/* count unique horizontal edges */
	h_edges = count_nodes(&region->top_edges);
	h_edges += count_nodes(&region->bottom_edges);
	
	/* count unique vertical edges */
	v_edges = count_nodes(&region->left_edges);
	v_edges += count_nodes(&region->right_edges);

	return region->area * (v_edges + h_edges);
}

static void region_free_edge_list(struct aoc_dlist_node *edge_list)
{
	struct aoc_dlist_node *node;
	struct edge *edge;
	assert(edge_list != NULL);
	for (node = edge_list->next; node != edge_list; ) {
		edge = aoc_dlist_container(node, offsetof(struct edge, node));
		node = node->next;
		aoc_dlist_remove(&edge->node);
		free(edge);
	}

	return;
}

static void region_free_edges(struct garden_region *region)
{
	assert(region != NULL);
	region_free_edge_list(&region->top_edges);
	region_free_edge_list(&region->right_edges);
	region_free_edge_list(&region->bottom_edges);
	region_free_edge_list(&region->left_edges);
	return;
}

static void free_region(struct garden_region *region)
{
	assert(region);
	region_free_edges(region);
	free(region);
	return;
}

int main(void)
{
	struct aoc_mapcache *garden;
	struct aoc_lut *plot_lut;
	struct aoc_dlist_node regions;
	int total_price = 0;

	garden = aoc_new_mapcache("input");
	plot_lut = aoc_new_lut(12, sizeof(unsigned long), sizeof(struct plot_info));
	assert(garden);
	assert(plot_lut);

	/* pass 1: for each unvisited tile we encounter... process the region
	 * where it belongs */
	aoc_mapcache_reset(garden);
	aoc_dlist_init(&regions);
	for (;;) {
		int err;
		unsigned long plot_id;
		int plant;
		struct plot_info plot_info;

		plant = aoc_mapcache_tile(garden, &plot_id);
		err = aoc_lut_lookup(plot_lut, &plot_id, sizeof plot_id,
				&plot_info, sizeof plot_info);
		if (err == -1) {
			struct garden_region *region;
			region = new_garden_region(plant);
			assert(region != NULL);
			aoc_dlist_prepend(&regions, &region->node);
			garden_plot_scan_region(garden, region, plot_lut);
			assert(aoc_lut_lookup(plot_lut, &plot_id, sizeof plot_id,
				&plot_info, sizeof plot_info) == 0);
		}
		assert(plot_info.region != NULL);
		if (aoc_mapcache_walk_forward(garden) == -1)
			break;
	}

	/* pass 2: for each tile, get its boundaries */
	aoc_mapcache_reset(garden);
	for (;;) {
		garden_plot_edges(garden, plot_lut);
		if (aoc_mapcache_walk_forward(garden) == -1)
			break;
	}

	struct garden_region *tmp;
	struct aoc_dlist_node *node;
	for (node = regions.next; node != &regions; ) {
		tmp = aoc_dlist_container(node, offsetof(struct garden_region, node));
		total_price += region_get_price(tmp);
		node = node->next;
		free_region(tmp);
	}

	printf("total price: %d\n", total_price);

	aoc_free_lut(plot_lut);
	aoc_free_mapcache(garden);
        return 0;
}

