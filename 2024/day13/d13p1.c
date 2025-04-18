#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stddef.h>

#include <aoc/die.h>
#include <aoc/lncache.h>
#include <aoc/dlist.h>

struct button_config {
	int x;
	int y;
	int cost;
};

struct prize {
	int x;
	int y;
};

struct move_code {
	struct button_config a;
	struct button_config b;
	struct prize prize;
	struct aoc_dlist_node node; /* link to next button_and_prize */
};

static void extract_to_buffer(struct aoc_lncache *cache,
	char *buffer, size_t buf_len, int idx)
{
	struct aoc_line *line;
	int ret;
	size_t line_len;
	assert(cache != NULL);
	assert(buffer != NULL);
	assert(buf_len > 0);
	assert(idx >= 0);
	
	ret = aoc_lncache_getline(cache, &line, idx);
	if (ret == -1) {
		assert(0);
	}
	line_len = aoc_line_strlen(line);
	assert(line_len < buf_len);
	aoc_line_get(line, buffer, buf_len);
	return;
}

static void button_extract(char *line, struct button_config *config, int cost)
{
	char button;
	int x;
	int y;
	assert(line != NULL);
	assert(config != NULL);
	sscanf(line, "Button %c: X+%d, Y+%d", &button, &x, &y);
	config->x = x;
	config->y = y;
	config->cost = cost;
	/*printf("Button %c -> X: %d, Y:%d\n", button, *x, *y);*/
	return;
}

static void prize_extract(char *line, struct prize *prize)
{
	int x;
	int y;
	assert(line != NULL);
	assert(prize != NULL);
	sscanf(line, "Prize: X=%d, Y=%d", &x, &y);
	prize->x = x;
	prize->y = y;
	/*printf("Prize->  X:%d, Y:%d\n", *x, *y);*/
	return;
}

static void append_new_cheat_code(struct aoc_dlist_node *cheat_code_list,
	char *button_a_line, char *button_b_line, char *prize_line)
{
	struct move_code *move_code;

	assert(cheat_code_list != NULL);
	assert(button_a_line != NULL);
	assert(button_b_line != NULL);
	assert(prize_line != NULL);

	move_code = malloc(sizeof * move_code);
	assert(move_code != NULL);
	
	button_extract(button_a_line, &move_code->a, 3);
	button_extract(button_b_line, &move_code->b, 1);
	prize_extract(prize_line, &move_code->prize);

	aoc_dlist_append(cheat_code_list->prev, &move_code->node);
	
	return;
}

static void create_cheat_code_list (
	struct aoc_lncache *button_config_file,
	struct aoc_dlist_node *move_codes)
{
	int line_count;
	int i;
	assert(button_config_file != NULL);
	assert(move_codes);

	/* cheat codes are in groups of three lines */
	line_count = aoc_lncache_line_count(button_config_file);
	assert(line_count > 0);
	for (i = 0; i < line_count; i += 3) {
		char line1[64]; /* button A */
		char line2[64]; /* button B */
		char line3[64]; /* prize */

		extract_to_buffer(button_config_file, line1, sizeof line1, i);
		extract_to_buffer(button_config_file, line2, sizeof line2, i+1);
		extract_to_buffer(button_config_file, line3, sizeof line3, i+2);
		append_new_cheat_code(move_codes, line1, line2, line3);
	}
	
	return;
}

static void simulate_button_press(struct button_config *config,
	int *x, int *y, int press)
{
	assert(config != NULL);
	assert(x != NULL);
	assert(y != NULL);
	*x += (config->x * press);
	*y += (config->y * press);
	return;
}

static int compute_smallest_token(struct move_code *move_code)
{
	int a_presses;
	int b_presses;
	for (a_presses = 0; a_presses < 100; a_presses += 1) {
		for (b_presses = 0; b_presses < 100; b_presses += 1) {
			int x = 0;
			int y = 0;

			simulate_button_press(&move_code->a, &x, &y, a_presses);
			simulate_button_press(&move_code->b, &x, &y, b_presses);

			if (x != move_code->prize.x) 
				continue;

			if (y != move_code->prize.y)
				continue;

			return (a_presses * move_code->a.cost) + 
				(b_presses * move_code->b.cost);
		}
	}
	return -1;
}

static int get_minimum_token(struct aoc_dlist_node *move_codes)
{
	struct aoc_dlist_node *node;
	int minimium_token = 0;
	assert(move_codes != NULL);
	for (node = move_codes->next ; node != move_codes; node = node->next) {
		struct move_code *move_code;
		int smallest_token;
		move_code = aoc_dlist_container(node, offsetof(struct move_code, node));
		smallest_token = compute_smallest_token(move_code);
		if (smallest_token != -1) {
			minimium_token += smallest_token;
		}
	}	
	return minimium_token;
}

static void release_all_move_codes(struct aoc_dlist_node *move_codes)
{
	struct aoc_dlist_node *node;
	assert(move_codes != NULL);
	for (node = move_codes->next ; node != move_codes;) {
		struct move_code *move_code;
		struct aoc_dlist_node *tmp;
		tmp = node->next;
		move_code = aoc_dlist_container(node, offsetof(struct move_code, node));
		free(move_code);
		node = tmp;
	}	
	return;
}

int main(void)
{
	struct aoc_lncache *button_config_file;
	struct aoc_dlist_node move_codes;
	int minimium_tokens;

	if ((button_config_file = aoc_new_lncache("input")) == NULL) {
		aoc_die(-1, "cannot open file [%s]\n", "input");
	}

	aoc_dlist_init(&move_codes);
	create_cheat_code_list(button_config_file, &move_codes);
	
	minimium_tokens = get_minimum_token(&move_codes);
	printf("token %d\n", minimium_tokens);

	release_all_move_codes(&move_codes);
	aoc_free_lncache(button_config_file);
	return 0;
}

