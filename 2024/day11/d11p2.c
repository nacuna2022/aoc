#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include <aoc/lncache.h>
#include <aoc/dlist.h>
#include <aoc/lut.h>

struct lut_data {
	unsigned long count;
};

static int value_digit_n(unsigned long value)
{
	int count = 0;
	while(value) {
		value /= 10;
		count += 1;
	}
	return count;
}

static bool stone_has_even_digits(unsigned long value)
{
	int count;
	count = value_digit_n(value);
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

static void split_stone(unsigned long value, unsigned long *value1,
	unsigned long *value2)
{
	int count;
	assert(value1 != NULL);
	assert(value2 != NULL);
	assert(stone_has_even_digits(value) == true);
	count = value_digit_n(value) >> 1;
	assert(count > 0);

	unsigned long divisor = unsigned_pow(10, count);
	*value1 = value / divisor;
	*value2 = value % divisor;
	return;
}

static bool lut_is_empty(struct aoc_lut *lut)
{
	size_t i = 0;
	struct aoc_lut_node *node;
	while(aoc_lut_node_idx(lut, &node, i) != -1) {
		if (node != NULL) {
			return false;
		}
		i += 1;
	}
	return true;
}

static void add_stone(struct aoc_lut *lut, unsigned long value,
	unsigned long count)
{
	struct aoc_lut_node *tmp;
	struct lut_data *tmp_data;
	tmp = aoc_lut_add(lut, value);
	tmp_data = aoc_lut_node_data(tmp);
	if (tmp_data->count == 0)
		tmp_data->count = count;
	else
		tmp_data->count += count;
	return;
}

/* process this stone according to the rules.
 * add new stones to the spare_lut as necessary */
void process_nodeptr(struct aoc_lut_node *nodeptr, struct aoc_lut *spare_lut)
{
	struct lut_data *lut_data = aoc_lut_node_data(nodeptr);

	if (nodeptr->key == 0) {
		add_stone(spare_lut, 1, lut_data->count);
	} else if (stone_has_even_digits(nodeptr->key) == true) {
		unsigned long value1;
		unsigned long value2;
		split_stone(nodeptr->key, &value1, &value2);
		add_stone(spare_lut, value1, lut_data->count);
		add_stone(spare_lut, value2, lut_data->count);
	} else {
		add_stone(spare_lut, nodeptr->key * 2024, lut_data->count);
	}
	return;
}

static void blink(struct aoc_lut *active_lut, struct aoc_lut *spare_lut)
{
	struct aoc_lut_node *nodeptr;
	size_t i = 0;
	/* traverse the active lut looking for stones to process. */
	while(aoc_lut_node_idx(active_lut, &nodeptr, i) != -1) {
		if (nodeptr != NULL) {
			while(nodeptr != NULL) {
				struct aoc_lut_node *next = nodeptr->link;
				process_nodeptr(nodeptr, spare_lut);
				
				/* remove this stone from the active_lut since
				 * it has been processed now. */
				aoc_lut_remove(nodeptr);
				nodeptr = next;	
			}

		}
		
		/* onto the next stone in lut */
		i++;
	}
	/* at the end of the loop, the active_lut should be empty */
	assert(lut_is_empty(active_lut));
	return ;
}

static void init_stone_lut(struct aoc_lut *lut,	struct aoc_line *line)
{
	char buffer[1024];
	char *endptr = buffer;
	unsigned long value;
	assert(aoc_line_strlen(line) <= sizeof buffer);
	aoc_line_get(line, buffer, sizeof buffer);
	while(*endptr != '\0') {
		value = strtoul(endptr, &endptr, 10);
		/* nacuna??? assume here digit in input is unique
		 * thats why we only add '1' count */
		add_stone(lut, value, 1);
		if (*endptr == ' ')
			endptr += 1;
	}
	return;
}

static unsigned long count_stones_in_node(struct aoc_lut_node *node)
{
	struct lut_data *lut_data = aoc_lut_node_data(node);
	return lut_data->count;
}

static unsigned long count_stones(struct aoc_lut *lut)
{
	unsigned long count = 0;
	size_t i = 0;
	struct aoc_lut_node *chain;
	while(aoc_lut_node_idx(lut, &chain, i) != -1) {
		if (chain != NULL) {
			struct aoc_lut_node *nodeptr = chain;
			while (nodeptr != NULL) {
				count += count_stones_in_node(nodeptr);
				nodeptr = nodeptr->link;
			}
		}
		i += 1;
	}
	return count;
}

int main(void)
{
	struct aoc_lncache *lncache;
	struct aoc_line *line;
	struct aoc_lut *lut[2] = {0};
	int blink_i;
	unsigned long count;

	assert((lncache = aoc_new_lncache("input")) != NULL);
	assert(aoc_lncache_getline(lncache, &line, 0) != -1);
	assert((lut[0] = aoc_new_lut(12, sizeof(struct lut_data), NULL)) != NULL);
	assert((lut[1] = aoc_new_lut(12, sizeof(struct lut_data), NULL)) != NULL);

	struct aoc_lut *active_lut = lut[0];
	struct aoc_lut *spare_lut = lut[1];
	init_stone_lut(active_lut, line);
	for (blink_i = 0; blink_i < 75; blink_i +=1) {
		struct aoc_lut *tmp = active_lut;
		blink(active_lut, spare_lut);
		active_lut = spare_lut;
		spare_lut = tmp;
	}
	assert(lut_is_empty(spare_lut));
	count = count_stones(active_lut);
	printf("stone count: %lu\n", count);
	aoc_free_lut(active_lut);
	aoc_free_lut(spare_lut);
	aoc_free_lncache(lncache);
	return 0;
}


