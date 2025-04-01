#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include <aoc/lncache.h>
#include <aoc/dlist.h>

struct stone {
	unsigned long value;
	struct aoc_dlist_node link;
};

static void init_stone_list(struct aoc_dlist_node *stone_list, 
	struct aoc_line *line)
{
	char buffer[1024];
	char *endptr = buffer;
	unsigned long value;
	assert(aoc_line_strlen(line) <= sizeof buffer);
	aoc_line_get(line, buffer, sizeof buffer);
	while(*endptr != '\0') {
		struct stone *stone;
		value = strtoul(endptr, &endptr, 10);
		stone = malloc(sizeof * stone);
		assert(stone != NULL);
		stone->value = value;
		aoc_dlist_prepend(stone_list, &stone->link);
		if (*endptr == ' ')
			endptr += 1;
	}
	return;
}

static int value_digit_n(unsigned long value)
{
	int count = 0;
	while(value) {
		value /= 10;
		count += 1;
	}
	return count;
}

static bool stone_has_even_digits(struct stone *stone)
{
	int count;
	assert(stone != NULL);
	assert(stone->value != 0);
	count = value_digit_n(stone->value);
	return count & 1 ? false : true;
}

static unsigned long unsigned_pow(unsigned long base, unsigned long exponent)
{
	unsigned long result = 1;
	while(exponent > 0) {
		result *= base;
		exponent -= 1;
	}
	return result;
}

static void split_stone(struct stone *stone1, struct stone **stone2pp)
{
	int count;
	assert(stone1 != NULL);
	assert(stone2pp != NULL);
	assert(stone_has_even_digits(stone1) == true);
	count = value_digit_n(stone1->value) >> 1;
	assert(count > 0);

	unsigned long divisor = unsigned_pow(10, count);
	unsigned long value = stone1->value;
	stone1->value = value / divisor;

	struct stone *stone2;
	stone2 = malloc(sizeof *stone2);
	assert(stone2 != NULL);
	stone2->value = value % divisor;

	*stone2pp = stone2;

	return;
}

static void blink(struct aoc_dlist_node *stone_list)
{
	struct aoc_dlist_node *ptr;
	for (ptr = stone_list->next; ptr != stone_list; ptr = ptr->next) {
		struct stone *stone;
		stone = aoc_dlist_container(ptr, offsetof(struct stone, link));
		
		/* rule 1: if a stone is 0, then replace it with 1 */
		if (stone->value == 0) {
			stone->value = 1;
			continue;
		}

		/* rule 2: if a stone has even numbered digits, split it */
		
		if (stone_has_even_digits(stone)) {
			struct stone *stone2 = NULL;
			split_stone(stone, &stone2);

			/* add new stone2 now to list */
			assert(stone2 != NULL);
			aoc_dlist_append(&stone->link, &stone2->link);

			/* and also skip it from the traversal */
			ptr = ptr->next;

			continue;
		}
		
		/* rule 3: if none of the rules apply, multiply by 2024 */
		stone->value = stone->value * 2024;
	}
	return;
}

#if 0
static void print_stone_list(struct aoc_dlist_node *stone_list)
{
	struct aoc_dlist_node *ptr;
	for (ptr = stone_list->next; ptr != stone_list; ptr = ptr->next) {
		struct stone *stone;
		stone = aoc_dlist_container(ptr, offsetof(struct stone, link));
		printf("%lu ", stone->value);
		fflush(stdout);
	}
	printf("\n");
	return;
}
#endif

static void release_stone_list(struct aoc_dlist_node *stone_list)
{
	struct aoc_dlist_node *tmp;
	assert(stone_list != NULL);
	for (tmp = stone_list->next; tmp != stone_list; ) {
		struct stone *stone;
		stone = aoc_dlist_container(tmp, offsetof(struct stone, link));
		tmp = stone->link.next;
		free(stone);
	}
	return;
}

static int count_stones(struct aoc_dlist_node *stone_list)
{
	int count = 0;
	struct aoc_dlist_node *tmp;
	assert(stone_list != NULL);
	for (tmp = stone_list->next; tmp != stone_list; tmp = tmp->next) {
		count += 1;
	}
	return count;
}

int main(void)
{
	struct aoc_lncache *lncache;
	struct aoc_line *line;
	int blink_count = 25;
	int stone_count;

	lncache = aoc_new_lncache("input");
	if (aoc_lncache_getline(lncache, &line, 0) != -1) {
		struct aoc_dlist_node stone_list;
		aoc_dlist_init(&stone_list);
		init_stone_list(&stone_list, line);
		while(blink_count) {
			blink(&stone_list);
			blink_count -= 1;
		}
		stone_count = count_stones(&stone_list);
		printf("stone count: %d\n", stone_count);
		release_stone_list(&stone_list);
	}
	aoc_free_lncache(lncache);
	return 0;
}

