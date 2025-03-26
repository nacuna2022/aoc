#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <aoc_input.h>
#include <aoc_die.h>

static struct aoc_input *aoc_process_input(struct aoc_input *aoc_input,
		int fd, size_t size)
{
	return NULL;
}

static struct aoc_input *aoc_new_input(int fd, size_t size)
{
	struct aoc_input *aoc_input;
	if ((aoc_input = malloc(sizeof * aoc_input)) == NULL) {
		aoc_die(-1, "cannot allocate memory for input\n");
	}
	return aoc_process_input(aoc_input, fd, size);
}

struct aoc_input *aoc_input_new_from_file(char *pathname)
{
	int fd;
	struct stat input_stat;
	size_t input_size;
	struct aoc_input aoc_input;
	if ((fd = open(pathname, O_RDONLY)) == -1) {
		aoc_die(-1, "cannot open file: %s\n", pathname);
	}
	if (fstat(fd, &input_stat) == -1) {
		aoc_die(-1, "cannot stat file: %s\n", pathname);
	}
	aoc_input = aoc_new_input(fd, input_stat.st_size);
	close(fd);
	return NULL;
}

void free_aoc_input(struct aoc_input *input)
{
	return; 
}

