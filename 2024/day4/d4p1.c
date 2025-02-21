#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

static int line_count;	/* row */
static int line_length; /* column */

struct map {
	char *buf;
	int pos;
	int line_count;
	int line_length;
} map;

static size_t get_line(int fd, char *buf, size_t buf_size)
{
	size_t count = 0;
	int err;
	char b;
	while((err = read(fd, &b, 1)) != 0 && (b != '\n')) {
		if (err == -1)
			exit(err);

		if (buf && (count < (buf_size))) {
			buf[count] = b;
		}

		count += 1;
	}
	return count;
}

/* !!!multiple passes on input file to get dimensions!!! */
static int init_map_from_pathname(struct map *map, char *pathname, 
		int *line_length, int *line_count)
{
	char *m;
	int fd;
	int err;
	int i;
	int llen = 0;
	int lcount = 0;
	
	if ((fd = open(pathname, O_RDONLY)) == -1) {
		return -1;
	}
	lseek(fd, 0, SEEK_SET);
	llen = get_line(fd, NULL, 0);

	lseek(fd, 0, SEEK_SET);
	while (get_line(fd, NULL, 0)) {
		lcount += 1;
	}

	m = calloc(lcount, llen);
	if (!m) {
		fprintf(stderr, "cannot allocate memory\n");
		exit(-1);
	}
	
	lseek(fd, 0, SEEK_SET);
	err = 0;
	for (i=0; i< lcount; i++) {
		int tmp = get_line(fd, m + (i * lcount), llen);
		assert(tmp != -1);
		err += tmp;
	}
	assert(err == lcount * llen);

	map->buf = m;
	map->line_length = llen;
	map->line_count = lcount;
	map->pos = 0;
	close(fd);
	return 0;
}

static void map_garbage_collect(struct map *map)
{
	free(map->buf);
	map->buf = NULL;
	return;
}

static inline int map_tile(struct map *map)
{
	return map->buf[map->pos];
}

/* basic bounds checking if we are stepping off our map */
static int map_step(struct map *map, int step)
{
	int err = -1;
	int new_pos = map->pos + step;
	int last_pos = (map->line_length * map->line_count);

	if ((new_pos >= 0) && (new_pos < last_pos)){
		map->pos = new_pos;
		return map_tile(map);
	}
	return err;
}

static int map_up(struct map *map)
{
	return map_step(map, -map->line_length);
}

static int map_down(struct map *map)
{
	return map_step(map, map->line_length);
}

/* for left stepping, we need to see if we are falling off the current row */
static int map_left(struct map *map)
{
	int new_pos = map->pos - 1;
	int lower_bound;

	/* lets round down to get the lower bound step */
	lower_bound = (map->pos / map->line_length) * map->line_length;
	return (new_pos >= lower_bound) ? map_step(map, -1) : -1;
}

/* see left stepping comment */
static int map_right(struct map *map)
{
	int new_pos = map->pos + 1;
	int upper_bound;

	/* lets round up to get the upper bound step */
	upper_bound = (map->pos + map->line_length) - (map->pos % map->line_length);
	return (new_pos < upper_bound) ? map_step(map, 1) : -1;
}

/* for diagonal stepping, its any variation of [up/down] + [left/right] */
static int map_up_right(struct map *map)
{
	int err;
	if ((err = map_up(map)) != -1) {
		if ((err = map_right(map)) != -1) {
			return err;
		}
		map_down(map);
		err = -1;
	}
	return err;
}

static int map_down_right(struct map *map)
{
	int err;
	if ((err = map_down(map)) != -1) {
		if ((err = map_right(map)) != -1) {
			return err;
		}
		map_up(map);
		err = -1;
	}
	return err;
}

static int map_up_left(struct map *map)
{
	int err;
	if ((err = map_up(map)) != -1) {
		if ((err = map_left(map)) != -1) {
			return err;
		}
		map_down(map);
		err = -1;
	}
	return err;
}

static int map_down_left(struct map *map)
{
	int err;
	if ((err = map_down(map)) != -1) {
		if ((err = map_left(map)) != -1) {
			return err;
		}
		map_up(map);
		err = -1;
	}
	return err;
}

static const char the_word[] = "XMAS";

static int get_0300_instance(struct map *map, const char *p)
{
	int err = 0;
	int ret;

	if (*p == '\0') {
		return 1;
	}

	if ((ret = map_right(map)) == *p) {
		err = get_0300_instance(map, p+1);
	}

	if (ret != -1) {
		map_left(map);
	}
	return err;
}

static int get_0600_instance(struct map *map, const char *p)
{
	int err = 0;
	int ret;

	if (*p == '\0') {
		return 1;
	}

	if ((ret = map_down(map)) == *p) {
		err = get_0600_instance(map, p+1);
	}

	if (ret != -1) {
		map_up(map);
	}
	return err;
}

static int get_0900_instance(struct map *map, const char *p)
{
	int err = 0;
	int ret;

	if (*p == '\0') {
		return 1;
	}

	if ((ret = map_left(map)) == *p) {
		err = get_0900_instance(map, p+1);
	}

	if (ret != -1) {
		map_right(map);
	}
	return err;
}

static int get_1200_instance(struct map *map, const char *p)
{
	int err = 0;
	int ret;

	if (*p == '\0') {
		return 1;
	}

	if ((ret = map_up(map)) == *p) {
		err = get_1200_instance(map, p+1);
	}

	if (ret != -1) {
		map_down(map);
	}
	return err;
}

static int get_0130_instance(struct map *map, const char *p)
{
	int err = 0;
	int ret;

	if (*p == '\0') {
		return 1;
	}

	if ((ret = map_up_right(map)) == *p) {
		err = get_0130_instance(map, p+1);
	}

	if (ret != -1) {
		map_down_left(map);
	}
	return err;
}

static int get_0430_instance(struct map *map, const char *p)
{
	int err = 0;
	int ret;

	if (*p == '\0') {
		return 1;
	}

	if ((ret = map_down_right(map)) == *p) {
		err = get_0430_instance(map, p+1);
	}

	if (ret != -1) {
		map_up_left(map);
	}
	return err;
}

static int get_0730_instance(struct map *map, const char *p)
{
	int err = 0;
	int ret;

	if (*p == '\0') {
		return 1;
	}

	if ((ret = map_down_left(map)) == *p) {
		err = get_0730_instance(map, p+1);
	}

	if (ret != -1) {
		map_up_right(map);
	}
	return err;
}

static int get_1030_instance(struct map *map, const char *p)
{
	int err = 0;
	int ret;

	if (*p == '\0') {
		return 1;
	}

	if ((ret = map_up_left(map)) == *p) {
		err = get_1030_instance(map, p+1);
	}

	if (ret != -1) {
		map_down_right(map);
	}
	return err;
}

static int get_instance_count(struct map *map)
{
	const char *p = the_word;
	int instance = 0;

	instance += get_1200_instance(map, p+1);
	instance += get_0130_instance(map, p+1);
	instance += get_0300_instance(map, p+1);
	instance += get_0430_instance(map, p+1);
	instance += get_0600_instance(map, p+1);
	instance += get_0730_instance(map, p+1);
	instance += get_0900_instance(map, p+1);
	instance += get_1030_instance(map, p+1);

	return instance;
}

int main(void)
{
	int tile;
	int instance = 0;
	init_map_from_pathname(&map, "input", &line_length, &line_count);

	/* we walk the entire map until we aren't able to */
	for (;;) {
		if ((tile = map_tile(&map)) == 'X') {
			instance += get_instance_count(&map);
		}
		tile = map_step(&map, 1);
		if (tile == -1) {
			break;
		}
	}

	printf("instance count: %d\n", instance);
	return 0;
}

