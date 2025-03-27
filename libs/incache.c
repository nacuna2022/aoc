#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <aoc/incache.h>

struct aoc_incache {
	char *raw;
	size_t size;	
};

static int aoc_cache_input(struct aoc_incache *aoc_input, int fd, size_t size)
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

static struct aoc_incache *aoc_incache_new(int fd, size_t size)
{
	struct aoc_incache *incache;
	if ((incache = malloc(sizeof * incache)) != NULL) {
		if ((aoc_cache_input(incache, fd, size)) != 0) {
			free(incache);
			incache = NULL;
		}
		incache->size = size;
	}
	return incache;
}

struct aoc_incache *aoc_new_incache(char *pathname)
{
	int fd;
	struct stat input_stat;
	struct aoc_incache *incache = NULL;
	if ((fd = open(pathname, O_RDONLY)) == -1) {
		goto open_nok;
	}
	if (fstat(fd, &input_stat) == -1) {
		goto open_ok;
	}
	incache = aoc_incache_new(fd, input_stat.st_size);
open_ok:
	close(fd);
open_nok:
	return incache;
}

void aoc_free_incache(struct aoc_incache *incache)
{
	assert(incache != NULL);
	free(incache->raw);
	free(incache);
	return; 
}

size_t aoc_incache_size(struct aoc_incache *incache)
{
	assert(incache != NULL);
	return incache->size;
}

int aoc_incache_get(struct aoc_incache *incache, int idx)
{
	int ret = -1;
	assert(incache != NULL);
	if (idx < incache->size) {
		ret = *(incache->raw + idx);
	}
	return ret;
}

