#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#include <aoc/die.h>
#include <aoc/lncache.h>
#include <aoc/dlist.h>

struct button_config {
	long long x;
	long long y;
	long long cost;
};

struct prize {
	long long x;
	long long y;
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

	long long x;
	long long y;
	assert(line != NULL);
	assert(config != NULL);
	sscanf(line, "Button %c: X+%lld, Y+%lld", &button, &x, &y);
	config->x = x;
	config->y = y;
	config->cost = cost;
	/*printf("Button %c -> X: %d, Y:%d\n", button, *x, *y);*/
	return;
}

static void prize_extract(char *line, struct prize *prize)
{
	long long x;
	long long y;
	static const long long offset = 10000000000000;
	assert(line != NULL);
	assert(prize != NULL);
	sscanf(line, "Prize: X=%lld, Y=%lld", &x, &y);
	prize->x = x + offset;
	prize->y = y + offset;
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
		char line1[128]; /* button A */
		char line2[128]; /* button B */
		char line3[128]; /* prize */

		extract_to_buffer(button_config_file, line1, sizeof line1, i);
		extract_to_buffer(button_config_file, line2, sizeof line2, i+1);
		extract_to_buffer(button_config_file, line3, sizeof line3, i+2);
		append_new_cheat_code(move_codes, line1, line2, line3);
	}
	
	return;
}

/* everytime we press a, claw moves by (aX, aY)
 * everytime we press b, class moves by (bX, bY)
 *
 * when we press a "m" times and press b "n" times, we get:
 *
 * final.X = m * a.X + n * b.X
 * final.Y = m * a.Y + n * b.Y
 *
 * and if final.X == prize.X && final.Y == prize.Y then we get a winner.
 *
 * lets try to eliminate "n":
 *
 * 1. multiply
 * 	final.X * b.Y = (m * a.X + n * b.X) * b.Y : multiply by b.Y
 * 	final.Y * b.X = (m * a.Y + n * b.Y) * b.X : multiply by b.X
 *
 * 2. distribute. becomes:
 * 	final.X * b.Y = m * a.X * b.Y + n * b.X * b.Y
 * 	final.Y * b.X = m * a.Y * b.X + n * b.Y * b.X
 *
 * 3. rearrange multiplication is commutative: 
 * 	final.X * b.Y = m * a.X * b.Y + n * b.X * b.Y
 * 	final.Y * b.Y = m * a.Y * b.X + n * b.X * b.Y
 *
 * 4. let Z = n * b.X & b.Y
 * 	final.X * b.Y = m * a.X * b.Y + Z
 * 	final.Y * b.X = m * a.Y * b.X + Z
 *
 * 5. Z cancels out:
 * 	final.X * b.Y - final.Y * b.X = (m * a.X * b.Y + z) - (m * a.Y * b.X + z)
 *
 * 6. distribute m
 * 	final.X * b.Y - final.Y * b.X = m * a.X * b.Y - m * a.Y * b.X
 *
 * 7. isolate m
 * 	final.X * b.Y - final.Y * b.X = m (a.X * b.Y - a.Y * b.X)
 *
 * 8. final form for m:
 * 	final.X * b.Y - final.Y * b.X = m
 * 	-----------------------------
 * 	    a.X * b.Y - a.Y * b.X
 */
static int compute_m(long long finalx, long long finaly, long long ax, long long ay,
	long long bx, long long by, long long *m)
{
	long long numerator;
	long long denominator;

	numerator = (finalx * by) - (finaly * bx);
	denominator = (ax * by) - (ay * bx);

	if ((numerator % denominator) != 0)
		return -1;

	if ((numerator / denominator) < 0)
		return -1;

	*m = numerator / denominator;

	return 0;
}

/* ok now lets try computing for n. again we have the ff equations: 
 * 	final.X = m * a.X + n * b.X
 * 	final.Y = m * a.Y + n * b.Y
 *
 * 1) using the first formula for final.X, try to cancel out m * a.X first
 * 	final.X - m * a.X = m * a.X + n * b.X - m * a.X
 *
 * 2) now try to cancel out bx to isolate n.
 * 	final.X - m * a.X = n * b.X
 * 	-----------------   -------
 * 	        b.X            b.X
 *
 * 3) finally, we have the ff equation for n:
 * 	final.X - m * a.X = n
 * 	-----------------
 * 	       b.X
 */
static int compute_n(long long finalx, long long m, long long ax,
	long long bx, long long *n)
{
	long long numerator;
	long long denominator;

	numerator = finalx - (m * ax);
	denominator = bx;

	if ((numerator % denominator) != 0)
		return -1;

	if ((numerator / denominator) < 0)
		return -1;

	*n = numerator / denominator;

	return 0;
}

static long long compute_smallest_token(struct move_code *move_code)
{
	long long m;
	long long n;

	if (compute_m(move_code->prize.x, move_code->prize.y, move_code->a.x, move_code->a.y, move_code->b.x, move_code->b.y, &m) == -1) {
		return -1;
	}

	if (compute_n(move_code->prize.x, m, move_code->a.x, move_code->b.x, &n) == -1) {
		return -1;
	}

	return (move_code->a.cost * m) + (move_code->b.cost * n);
}

static long long get_minimum_token(struct aoc_dlist_node *move_codes)
{
	struct aoc_dlist_node *node;
	long long minimium_token = 0;
	assert(move_codes != NULL);
	for (node = move_codes->next ; node != move_codes; node = node->next) {
		struct move_code *move_code;
		long long smallest_token;
		move_code = aoc_dlist_container(node, offsetof(struct move_code, node));
		smallest_token = compute_smallest_token(move_code);
		if (smallest_token != -1) {
			minimium_token += smallest_token;
		}
		//printf("smallest token: %lld\n", smallest_token);
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
	long long minimium_tokens;

	if ((button_config_file = aoc_new_lncache("input")) == NULL) {
		aoc_die(-1, "cannot open file [%s]\n", "input");
	}

	aoc_dlist_init(&move_codes);
	create_cheat_code_list(button_config_file, &move_codes);
	
	minimium_tokens = get_minimum_token(&move_codes);
	printf("token %lld\n", minimium_tokens);

	release_all_move_codes(&move_codes);
	aoc_free_lncache(button_config_file);
	return 0;
}

