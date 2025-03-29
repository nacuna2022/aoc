#ifndef __AOC_INPUT_CACHE_H__
#define __AOC_INTPU_CACHE_H__
#include <stddef.h>

/* note to self: if you are interested in just getting raw characters 
 * then the input cache is the one to use. */

struct aoc_incache;

struct aoc_incache *aoc_new_incache(char *pathname);
void aoc_free_incache(struct aoc_incache *incache);
size_t aoc_incache_size(struct aoc_incache *incache);
int aoc_incache_get(struct aoc_incache *input_cache, size_t idx);

#endif /* __AOC_INPUT_CACHE_H__ */

