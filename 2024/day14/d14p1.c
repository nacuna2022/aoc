#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#include <aoc/lncache.h>
#include <aoc/mapcache.h>
#include <aoc/die.h>
#include <aoc/bot.h>
#include <aoc/dlist.h>

struct coord {
	int x;
	int y;
};

/* for reference, we assign the quadrants like so:
 *  0 | 1
 *  --+--
 *  3 | 2
 */
struct quadrant_coords {
	struct coord min;
	struct coord max;
};

struct robot_struct {
	struct aoc_bot *bot;
	struct coord position;
	struct coord velocity;
	struct coord final; /* x,y after simulation */
	int (*move_horizontal)(struct aoc_mapcache *);
	int (*move_vertical)(struct aoc_mapcache *);
	struct aoc_dlist_node node; /* next robot */
};

struct robot_struct *new_robot(int px, int py, int vx, int vy)
{
	struct robot_struct *robot;
	struct aoc_bot *bot;
	enum aoc_direction bot_direction;

	bot_direction = aoc_direction_right;
	if (vx < 0) {
		bot_direction = aoc_direction_left;
	}

	robot = malloc(sizeof * robot);
	assert(robot != NULL);
	
	bot = aoc_new_bot(bot_direction);
	assert(bot != NULL);
	robot->bot = bot;
	robot->position.x = px;
	robot->position.y = py;
	robot->velocity.x = abs(vx);
	robot->velocity.y = abs(vy);

	robot->move_horizontal = aoc_mapcache_warp_right;
	if(vx < 0) {
		robot->move_horizontal = aoc_mapcache_warp_left;
	}

	robot->move_vertical = aoc_mapcache_warp_down;
	if (vy < 0) {
		robot->move_vertical = aoc_mapcache_warp_up;
	}

	return robot;
}

static void free_robot(struct robot_struct *robot)
{
	assert(robot != NULL);
	assert(robot->bot != NULL);
	aoc_free_bot(robot->bot);
	free(robot);
	return;
}

static void add_new_robot(struct aoc_dlist_node *robot_list,
	struct aoc_line *robot_line)
{
	char buffer[256];
	size_t line_len;
	int px;
	int py;
	int vx;
	int vy;
	struct robot_struct *r;
	assert(robot_list != NULL);
	assert(robot_line != NULL);
	line_len = aoc_line_strlen(robot_line);
	assert(line_len < sizeof buffer);
	aoc_line_get(robot_line, buffer, sizeof buffer);
	sscanf(buffer, "p=%d,%d v=%d,%d", &px, &py, &vx, &vy);
	//printf("p=%d,%d v=%d,%d\n", px, py, vx, vy);

	r = new_robot(px, py, vx, vy);
	assert(r != NULL);

	aoc_dlist_append(robot_list->prev, &r->node);

	return;
}

static void create_robot_list(struct aoc_lncache *robot_input,
	struct aoc_dlist_node *robot_list)
{
	size_t i;
	size_t robot_line_count;
	assert(robot_input != NULL);
	assert(robot_list != NULL);
	robot_line_count = aoc_lncache_line_count(robot_input);
	for (i = 0; i < robot_line_count; i += 1) {
		struct aoc_line *robot_line;
		aoc_lncache_getline(robot_input, &robot_line, i);
		add_new_robot(robot_list, robot_line);
	}
	return;
}

static void release_all_robots(struct aoc_dlist_node *robot_list)
{
	struct aoc_dlist_node *node;
	assert(robot_list != NULL);
	for (node = robot_list->next; node != robot_list; ) {
		struct aoc_dlist_node *tmp = node->next;
		struct robot_struct *robot;
		robot = aoc_dlist_container(node, offsetof(struct robot_struct, node));
		free_robot(robot);
		node = tmp;
	}

	return;
}

static void move_horizontal(struct aoc_mapcache *ebhq,
	struct robot_struct *robot)
{
	int i;
	assert(ebhq != NULL);
	assert(robot != NULL);
	assert(robot->move_horizontal != NULL);
	for (i = 0; i < robot->velocity.x; i += 1) {
		robot->move_horizontal(ebhq);
	}
	return;
}

static void move_vertical(struct aoc_mapcache *ebhq,
	struct robot_struct *robot)
{
	int i;
	assert(ebhq != NULL);
	assert(robot != NULL);
	assert(robot->move_vertical != NULL);
	for (i = 0; i < robot->velocity.y; i += 1) {
		robot->move_vertical(ebhq);
	}
	return;
}

static void move_robot(struct aoc_mapcache *ebhq, struct robot_struct *robot)
{
	assert(ebhq != NULL);
	assert(robot != NULL);
	move_horizontal(ebhq, robot);
	move_vertical(ebhq, robot);
	return;
}

static void robot_set_start_position(struct aoc_mapcache *ebhq,
	struct robot_struct *robot)
{
	int i;
	assert(ebhq != NULL);
	assert(robot != NULL);

	aoc_mapcache_reset(ebhq);

	/* x is horizontal */
	for (i = 0; i < robot->position.x; i += 1) 
		aoc_mapcache_step_right(ebhq);

	/* y is vertical */
	for (i = 0; i < robot->position.y; i += 1)
		aoc_mapcache_step_down(ebhq);

	return;
}

static void robot_set_final_position(struct aoc_mapcache *ebhq,
	struct robot_struct *robot)
{
	int x;
	int y;
	assert(ebhq != NULL);
	assert(robot != NULL);
	aoc_mapcache_coord(ebhq, &x, &y);
	robot->final.x = x;
	robot->final.y = y;
	return;
}

static int robot_get_quadrant(struct aoc_mapcache *ebhq,
	struct robot_struct *robot);

static void simulate_one_robot(struct aoc_mapcache *ebhq,
	struct robot_struct *robot, int total_seconds)
{
	int seconds;
	assert(ebhq != NULL);
	assert(robot != NULL);
	assert(total_seconds > 0);
	robot_set_start_position(ebhq, robot);
	for (seconds = 0; seconds < total_seconds; seconds += 1) {
		move_robot(ebhq, robot);
	}
	robot_set_final_position(ebhq, robot);
	return;
}

static void simulate_all_robots(struct aoc_mapcache *ebhq,
	struct aoc_dlist_node *robot_list, int total_seconds)
{
	struct aoc_dlist_node *node;

	assert(ebhq != NULL);
	assert(robot_list != NULL);
	for (node = robot_list->next; node != robot_list; node = node->next) {
		struct robot_struct *robot;
		robot = aoc_dlist_container(node, offsetof(struct robot_struct, node));
		assert(robot != NULL);

		simulate_one_robot(ebhq, robot, total_seconds);
	}
	return;
}

static void set_quadrant0_coords(struct aoc_mapcache *ebhq,
	struct quadrant_coords *quadrant)
{
	assert(ebhq != NULL);
	assert(quadrant != NULL);
	
	quadrant->min.x = 0;
	quadrant->min.y = 0;
	quadrant->max.x = (aoc_mapcache_height(ebhq) >> 1);
	quadrant->max.y = (aoc_mapcache_width(ebhq) >> 1);
	return;
}

static void set_quadrant1_coords(struct aoc_mapcache *ebhq,
	struct quadrant_coords *quadrant)
{
	assert(ebhq != NULL);
	assert(quadrant != NULL);

	quadrant->min.x = 0;
	quadrant->min.y = (aoc_mapcache_width(ebhq) >> 1) + 1;
	quadrant->max.x = aoc_mapcache_height(ebhq) >> 1;
	quadrant->max.y = aoc_mapcache_width(ebhq);

	return;
}

static void set_quadrant2_coords(struct aoc_mapcache *ebhq,
	struct quadrant_coords *quadrant)
{
	assert(ebhq != NULL);
	assert(quadrant != NULL);

	quadrant->min.x = (aoc_mapcache_height(ebhq) >> 1) + 1;
	quadrant->min.y = (aoc_mapcache_width(ebhq) >> 1) + 1;
	quadrant->max.x = aoc_mapcache_height(ebhq);
	quadrant->max.y = aoc_mapcache_width(ebhq);
	return;
}

static void set_quadrant3_coords(struct aoc_mapcache *ebhq,
	struct quadrant_coords *quadrant)
{
	assert(ebhq != NULL);
	assert(quadrant != NULL);

	quadrant->min.x = (aoc_mapcache_height(ebhq) >> 1) + 1;
	quadrant->min.y = 0;
	quadrant->max.x = aoc_mapcache_height(ebhq);
	quadrant->max.y = (aoc_mapcache_width(ebhq) >> 1);
	return;
}

static bool robot_in_quadrant(struct robot_struct *robot,
	struct quadrant_coords *quad_coords)
{
	assert(robot != NULL);
	assert(quad_coords != NULL);

	if (    (robot->final.x >= quad_coords->min.x) &&
		(robot->final.x < quad_coords->max.x)) {
		if (    (robot->final.y >= quad_coords->min.y) &&
			(robot->final.y < quad_coords->max.y)) {
			return true;
		}
	}
	return false;
}

static int robot_get_quadrant(struct aoc_mapcache *ebhq,
	struct robot_struct *robot)
{
	static struct quadrant_coords q0 = {0};
	static struct quadrant_coords q1 = {0};
	static struct quadrant_coords q2 = {0};
	static struct quadrant_coords q3 = {0};
	static bool quadrant_coords_set = false;

	if (quadrant_coords_set == false) {
		assert(ebhq != NULL);
		set_quadrant0_coords(ebhq, &q0);
		set_quadrant1_coords(ebhq, &q1);
		set_quadrant2_coords(ebhq, &q2);
		set_quadrant3_coords(ebhq, &q3);
		quadrant_coords_set = true;
	}
	
	assert(robot != NULL);
	if (robot_in_quadrant(robot, &q0) == true)
		return 0;
	
	if (robot_in_quadrant(robot, &q1) == true)
		return 1;

	if (robot_in_quadrant(robot, &q2) == true)
		return 2;

	if (robot_in_quadrant(robot, &q3) == true)
		return 3;

	return -1;
}

static int lab_safety_factor(struct aoc_mapcache *ebhq, struct aoc_dlist_node *robot_list)
{
	int robot_counts_per_quadrant[4] = {0,0,0,0};
	struct aoc_dlist_node *node;
	assert(robot_list != NULL);
	for (node = robot_list->next; node != robot_list; node = node->next) {
		struct robot_struct *robot;
		int quad_index;
		robot = aoc_dlist_container(node, offsetof(struct robot_struct, node));
		quad_index = robot_get_quadrant(ebhq, robot);
		switch(quad_index) {
		case 0:
		case 1:
		case 2:
		case 3:
			robot_counts_per_quadrant[quad_index] += 1;
			break;
		default:
			/* nothing */
		}
	}

	return robot_counts_per_quadrant[0] * robot_counts_per_quadrant[1] *
		robot_counts_per_quadrant[2] * 	robot_counts_per_quadrant[3];
}

int main()
{
	struct aoc_lncache *robot_input;
	struct aoc_dlist_node robot_list;
	struct aoc_mapcache *ebhq;
	int safety_factor;

	robot_input = aoc_new_lncache("input");
	aoc_dlist_init(&robot_list);
	create_robot_list(robot_input, &robot_list);

	ebhq = aoc_new_mapcache_grid(103, 101, '.');
	assert(ebhq != NULL);

	simulate_all_robots(ebhq, &robot_list, 100);
	safety_factor = lab_safety_factor(ebhq, &robot_list);
	printf("Lab safety factor: %d\n", safety_factor);

	release_all_robots(&robot_list);
	aoc_free_lncache(robot_input);
	aoc_free_mapcache(ebhq);
	return 0;
}

