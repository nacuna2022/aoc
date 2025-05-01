#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include <aoc/lncache.h>
#include <aoc/dlist.h>
#include <aoc/lut.h>

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

static void add_stone(struct aoc_lut *lut, unsigned long key, 
	unsigned long count)
{
	unsigned long old_count;
	if (aoc_lut_lookup(lut, &key, sizeof key, &old_count, sizeof old_count) == 0) {
		/* we already have this stone, remove it 
		 * so we can update it and put it back */
		aoc_lut_remove(lut, &key, sizeof key);
		count += old_count;
	}

	/* now we can add our stone again to the lut */
	assert(aoc_lut_add(lut, &key, sizeof key, &count, sizeof count) == 0);
	return;
}

/* process this stone according to the rules.
 * add new stones to the spare_lut as necessary */
void process_stone(const void *_key, const void *_data, void *cb_param) //struct aoc_lut *spare_lut)
{
	unsigned long key = *((const unsigned long *)_key);
	const unsigned long count = *((const unsigned long *)_data);
	struct aoc_lut *spare_lut = (struct aoc_lut *)cb_param;
	if (key == 0) {
		add_stone(spare_lut, 1, count);
	} else if (stone_has_even_digits(key) == true) {
		unsigned long value1;
		unsigned long value2;
		split_stone(key, &value1, &value2);
		add_stone(spare_lut, value1, count);
		add_stone(spare_lut, value2, count);
	} else {
		add_stone(spare_lut, key * 2024, count);
	}
	return;
}

static struct aoc_lut *blink(struct aoc_lut *active_lut)
{
	struct aoc_lut *spare_lut;
	spare_lut = aoc_new_lut(12, sizeof(unsigned long), sizeof(unsigned long));
	assert(spare_lut != NULL);
	aoc_lut_foreach(active_lut, process_stone, spare_lut);
	return spare_lut;
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
		add_stone(lut, value, 1);
		if (*endptr == ' ')
			endptr += 1;
	}
	return;
}

static void node_count_stone(const void *key, const void *data, void *param)
{
	unsigned long count;
	unsigned long *update;
	(void)key;

	update = (unsigned long *)param;
	count = *(const unsigned long *)data;
	*update += count;
	return;
}

static unsigned long count_stones(struct aoc_lut *lut)
{
	unsigned long count = 0;
	aoc_lut_foreach(lut, node_count_stone, &count);
	return count;
}

int main(void)
{
	struct aoc_lncache *lncache;
	struct aoc_line *line;
	struct aoc_lut *lut;
	int blink_i;
	unsigned long count;

	assert((lncache = aoc_new_lncache("input")) != NULL);
	assert(aoc_lncache_getline(lncache, &line, 0) != -1);
	assert((lut = aoc_new_lut(2, sizeof(unsigned long), sizeof(unsigned long))) != NULL);
	init_stone_lut(lut, line);
	for (blink_i = 0; blink_i < 75; blink_i +=1) {
		struct aoc_lut *tmp;
		tmp = blink(lut);
		aoc_free_lut(lut);
		lut = tmp;
	}
	count = count_stones(lut);
	printf("stone count: %lu\n", count);
	aoc_free_lut(lut);
	aoc_free_lncache(lncache);
	return 0;
}


