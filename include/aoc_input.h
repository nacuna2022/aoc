#ifndef __AOC_INPUT_H__
#define __AOC_INTPU_H__
#include <limits.h>
#include <stddef.h>

struct aoc_input {
	char filepath[PATH_MAX];
	char *raw;
	size_t size;	
};

struct aoc_input *aoc_input_new_from_file(char *pathname);
void free_aoc_input(struct aoc_input *input);

#endif /* __AOC_INPUT_H__ */

