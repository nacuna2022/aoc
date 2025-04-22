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

struct robot_struct {
	struct aoc_bot *bot;
	struct coord position;
	struct coord velocity;
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

	aoc_mapcache_change_tile(ebhq, '.');

	return;
}

static void robot_update_position(struct aoc_mapcache *ebhq,
	struct robot_struct *robot) 
{
	assert(ebhq != NULL);
	assert(robot != NULL);
	aoc_mapcache_coord(ebhq, &robot->position.y, &robot->position.x);
	aoc_mapcache_change_tile(ebhq, '*');
	return;
}

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
	robot_update_position(ebhq, robot);
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

static unsigned long long compute_2d_variance(struct aoc_dlist_node *robot_list)
{
	struct aoc_dlist_node *node;
	unsigned long long sumx = 0;
	unsigned long long sumx_squared = 0;
	unsigned long long sumy = 0;
	unsigned long long sumy_squared = 0;
	unsigned long long count = 0;

	assert(robot_list != NULL);
	for (node = robot_list->next; node != robot_list; node = node->next) {
		struct robot_struct *robot;
		robot = aoc_dlist_container(node, offsetof(struct robot_struct, node));
		assert(robot != NULL);
		
		sumx += robot->position.x;
		sumx_squared += (robot->position.x * robot->position.x);
		sumy += robot->position.y;
		sumy_squared += (robot->position.y * robot->position.y);
		count += 1;
	}

	unsigned long long xvariance;
	unsigned long long yvariance;

	xvariance = (count * sumx_squared - sumx * sumx) / (count * (count-1));
	yvariance = (count * sumy_squared - sumy * sumy) / (count * (count-1));

	return xvariance + yvariance;
}

int main()
{
	struct aoc_lncache *robot_input;
	struct aoc_dlist_node robot_list;
	struct aoc_mapcache *ebhq;
	struct aoc_mapcache *ebhq_snapshot = NULL;

	robot_input = aoc_new_lncache("input");
	aoc_dlist_init(&robot_list);
	create_robot_list(robot_input, &robot_list);

	ebhq = aoc_new_mapcache_grid(103, 101, '.');
	assert(ebhq != NULL);

	int i;
	unsigned long long smallest_variance ;
	unsigned long long variance_2d;
	int time_step;
	variance_2d = compute_2d_variance(&robot_list);
	smallest_variance = variance_2d;
	for (i = 0; i < 7500; i += 1) {
		variance_2d = compute_2d_variance(&robot_list);
		if (variance_2d < smallest_variance) {
			if (ebhq_snapshot != NULL)
				aoc_free_mapcache(ebhq_snapshot);
			ebhq_snapshot = aoc_mapcache_dup(ebhq);
			smallest_variance = variance_2d;
			time_step = i;
		}

		simulate_all_robots(ebhq, &robot_list, 1);
	}
	printf("possible easter egg appearance at : %d, displaying map...", time_step);
	aoc_mapcache_show(ebhq_snapshot);
	printf("\n");
	
	release_all_robots(&robot_list);
	aoc_free_lncache(robot_input);
	aoc_free_mapcache(ebhq_snapshot);
	aoc_free_mapcache(ebhq);
	return 0;
}

