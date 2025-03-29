#ifndef __AOC_LINE_CACHE_H__
#define __AOC_LINE_CACHE_H__
#include <stddef.h>

struct aoc_lncache;
struct aoc_line;

struct aoc_lncache *aoc_new_lncache(char *pathname);
void aoc_free_lncache(struct aoc_lncache *lncache);
size_t aoc_lncache_line_count(struct aoc_lncache *lncache);
int aoc_lncache_getline(struct aoc_lncache *lncache, struct aoc_line **line, 
	size_t idx);
void aoc_lncache_print(struct aoc_lncache *lncache);

size_t aoc_line_strlen(struct aoc_line *line);
int aoc_line_get(struct aoc_line *line, char *buffer, size_t buffer_len);

#endif /* __AOC_LINE_CACHE_H__ */

