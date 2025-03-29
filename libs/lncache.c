#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <aoc/lncache.h>
#include <aoc/incache.h>

struct aoc_lncache {
	size_t count;
	size_t size;
	struct aoc_line *lines[];
};

struct aoc_line {
	size_t count;
	size_t size;
	char data[];
};

static struct aoc_lncache *new_lncache(void)
{
	struct aoc_lncache *lncache;
	lncache = malloc(sizeof * lncache);
	if (lncache != NULL) {
		lncache->count = 0;
		lncache->size = 0;
	}
	return lncache;
}

static struct aoc_line *new_aoc_line(void)
{
	struct aoc_line *line;
	line = malloc(sizeof *line);
	if (line) {
		line->count = 0;
		line->size = 0;
	}
	return line;
}

static int enlarge_lncache(struct aoc_lncache **lncache)
{
	static const size_t grow_factor = 32;
	struct aoc_lncache *ptr = *lncache;
	while (ptr->count >= ptr->size) {
		struct aoc_lncache *tmp;
		size_t new_size = (ptr->size + grow_factor) * sizeof(struct aoc_line **);
		tmp = realloc(ptr, new_size + sizeof(struct aoc_lncache));
		if (tmp == NULL) {
			return -1;
		}
		tmp->size += grow_factor;
		ptr = tmp;
	}
	*lncache = ptr;
	return 0;
}

static int enlarge_line(struct aoc_line **line)
{
	static const size_t grow_factor = 64;
	struct aoc_line *ptr = *line;
	while(ptr->count >= ptr->size) {
		struct aoc_line *tmp;
		size_t new_size = (ptr->size + grow_factor) * sizeof(char);
		tmp = realloc(ptr, new_size + sizeof(struct aoc_line));
		if (tmp == NULL) {
			return -1;
		}
		tmp->size += grow_factor;
		ptr = tmp;
	}
	*line = ptr;
	return 0;
}

struct aoc_lncache *aoc_new_lncache(char *pathname)
{
	struct aoc_incache *incache;
	struct aoc_lncache *lncache;
	struct aoc_line **line = NULL;
	int idx = -1;
	
	incache = aoc_new_incache(pathname);
	if (incache == NULL) {
		return NULL;
	}

	lncache = new_lncache();
	if (lncache == NULL) {
		goto release_incache;
	}

	for (;;) {
		int ch;
		idx += 1;
		ch = aoc_incache_get(incache, idx);
		if (ch == -1)
			break;

		if (ch == '\n') {
			line = NULL;
			continue;
		}

		/* if we reach here, we have a new line OR continuing with
		 * the current line. */
		if (line == NULL) {
			/* we are creating a new line. see if we need to alloc
			 * some more or if we still have enough */
			if (lncache->count >= lncache->size) {
				if (enlarge_lncache(&lncache) == -1) {
					aoc_free_lncache(lncache);
					lncache = NULL;
					goto release_incache;
				}
			}
			line = &lncache->lines[lncache->count];
			lncache->count += 1;
		
			/* create a record for this new line */
			struct aoc_line *newline;
			newline = new_aoc_line();
			if (newline == NULL) {
				aoc_free_lncache(lncache);
				lncache = NULL;
				goto release_incache;
			}

			newline->count = 0;
			newline->size = 0;
			*line = newline;

		}

		/* we have a line pointer at this point. see if we 
		 * can add this new character to it. */
		if ((*line)->count >= (*line)->size) {
			if (enlarge_line(line) == -1) {
				aoc_free_lncache(lncache);
				lncache = NULL;
				goto release_incache;
			}
		}
		/* finally, add the character to the line */
		(*line)->data[(*line)->count] = (char)ch;
		(*line)->count += 1;
	}
release_incache:
	aoc_free_incache(incache);
	return lncache;
}

static void for_each_aoc_line(struct aoc_lncache *lncache,
	void (*func)(struct aoc_line *))
{
	size_t i;
	struct aoc_line **line;
	assert(lncache != NULL);
	assert(func != NULL);

	line = lncache->lines;
	for (i = 0; i < lncache->count; i++) {
		struct aoc_line *ptr = *(line + i);
		assert(ptr != NULL);
		func(ptr);
	}
	return;
}

static void free_aoc_line(struct aoc_line *line)
{
	free(line);
	return;
}

void aoc_free_lncache(struct aoc_lncache *lncache)
{
	for_each_aoc_line(lncache, free_aoc_line);
	free(lncache);
	return;
}

void aoc_lncache_print_line(struct aoc_line *line)
{
	size_t i;
	assert(line != NULL);
	for(i = 0; i < line->count; i++) {
		putchar(line->data[i]);
	}
	putchar('\n');
	return;
}

void aoc_lncache_print(struct aoc_lncache *lncache)
{
	for_each_aoc_line(lncache, aoc_lncache_print_line);
	return;
}

size_t aoc_lncache_line_count(struct aoc_lncache *lncache)
{
	assert(lncache != NULL);
	return lncache->count;
}

int aoc_lncache_getline(struct aoc_lncache *lncache, struct aoc_line **line,
	size_t idx)
{
	int ret = -1;
	assert(lncache != NULL);
	assert(line != NULL);
	if (idx < lncache->count) {
		*line = lncache->lines[idx];
		ret = 0;
	}
	return ret;
}

size_t aoc_line_strlen(struct aoc_line *line)
{
	assert(line);
	return line->count + 1; /* including the NULL terminator */
}

int aoc_line_get(struct aoc_line *line, char *buffer, size_t buffer_len)
{
	assert(line != NULL);
	assert(buffer != NULL);
	if (buffer_len < (line->count + 1)) {
		return -1;
	}
	memcpy(buffer, line->data, line->count + 1);
	buffer[line->count] = '\0';
	return 0;
}

