#ifndef __AOC_MAP_H__
#define __AOC_MAP_H__
#include <stddef.h>

/* advent of code uses a lot of maps where we need to 
 * process up, down, left and right.
 */

struct aoc_map;

struct aoc_map *aoc_new_map(char *path_name);
void aoc_free_map(struct aoc_map *map);

int aoc_map_peek_up(struct aoc_map *map, char *tile);
int aoc_map_peek_right(struct aoc_map *map, char *tile);
int aoc_map_peek_down(struct aoc_map *map, char *tile);
int aoc_map_peek_left(struct aoc_map *map, char *tile);

int aoc_map_go_up(struct aoc_map *map, char *tile);
int aoc_map_go_right(struct aoc_map *map, char *tile);
int aoc_map_go_down(struct aoc_map *map, char *tile);
int aoc_map_go_left(struct aoc_map *map, char *tile);

#endif /* __AOC_MAP_H__ */
