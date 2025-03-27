#ifndef __AOC_INPUT_CACHE_H__
#define __AOC_INTPU_CACHE_H__
#include <stddef.h>

/* note to self: if you are interested in just getting raw characters 
 * then the input cache is the one to use. */

struct aoc_input_cache;

struct aoc_input_cache *aoc_new_input_cache_from_file(char *pathname);
void aoc_free_input_cache(struct aoc_input_cache *input_cache);
size_t aoc_input_cache_size(struct aoc_input_cache *input_cache);
int aoc_input_cache_get_element(struct aoc_input_cache *input_cache,
		int idx);

#endif /* __AOC_INPUT_CACHE_H__ */

