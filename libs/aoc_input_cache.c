#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <aoc_input_cache.h>
#include <aoc_die.h>

struct aoc_input_cache {
	char *raw;
	size_t size;	
};

static int aoc_cache_input(struct aoc_input_cache *aoc_input, int fd, 
		size_t size)
{
	char *cache;
	int ret = 0;

	if ((cache = malloc(size)) != NULL) {
		ssize_t count;
		char *ptr = cache;
		while((count = read(fd, ptr, size)) < size) {
			if (count == 0) {
				break;
			} else if (count == -1) {
				free(cache);
				ret = -1;
				break;
			}
			ptr += count;
			size -= count;
		}
		aoc_input->raw = cache;
	}
	return ret;
}

static struct aoc_input_cache *aoc_new_input_cache(int fd, size_t size)
{
	struct aoc_input_cache *input_cache;
	if ((input_cache = malloc(sizeof * input_cache)) != NULL) {
		if ((aoc_cache_input(input_cache, fd, size)) != 0) {
			free(input_cache);
			input_cache = NULL;
		}
		input_cache->size = size;
	}
	return input_cache;
}

struct aoc_input_cache *aoc_new_input_cache_from_file(char *pathname)
{
	int fd;
	struct stat input_stat;
	struct aoc_input_cache *input_cache = NULL;
	if ((fd = open(pathname, O_RDONLY)) == -1) {
		goto open_nok;
	}
	if (fstat(fd, &input_stat) == -1) {
		goto open_ok;
	}
	input_cache = aoc_new_input_cache(fd, input_stat.st_size);
open_ok:
	close(fd);
open_nok:
	return input_cache;
}

void free_aoc_input_cache(struct aoc_input_cache *input_cache)
{
	assert(input_cache != NULL);
	free(input_cache->raw);
	free(input_cache);
	return; 
}

size_t aoc_input_cache_size(struct aoc_input_cache *input_cache)
{
	assert(input_cache != NULL);
	return input_cache->size;
}

int aoc_input_cache_get_element(struct aoc_input_cache *input_cache,
		int idx)
{
	int ret = -1;
	assert(input_cache != NULL);
	if (idx < input_cache->size) {
		ret = *(input_cache->raw + idx);
	}
	return ret;
}

