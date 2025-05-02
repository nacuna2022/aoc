#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

struct disk_struct {
#define ENLARGE_SIZE	4
	int *blk; /* -1 file_id means blk[idx] is a free block */
	size_t blk_count;
	size_t capacity;
};

static void disk_enlarge_disk(struct disk_struct *disk)
{
	size_t new_capacity = disk->capacity + ENLARGE_SIZE;
	disk->blk = realloc(disk->blk, new_capacity * sizeof * disk->blk);
	assert(disk->blk != NULL);
	disk->capacity = new_capacity;
	return;
}

static bool disk_has_enough_space(struct disk_struct *disk, size_t count)
{
	return (disk->capacity - disk->blk_count) > count;
}

static void disk_append_blk(struct disk_struct *disk, int file_id,
		size_t count)
{
	size_t i;
	while(!disk_has_enough_space(disk, count)) {
		disk_enlarge_disk(disk);
	}
	for (i = 0; i < count; i++) {
		disk->blk[disk->blk_count + i] = file_id;
	}
	disk->blk_count += i;
	return;
}

static void disk_append_file(struct disk_struct *disk, int file_id,
		int count)
{
	assert(file_id >= 0);
	disk_append_blk(disk, file_id, count);
	return;
}

static void disk_append_free_blk(struct disk_struct *disk, int count)
{
	disk_append_blk(disk, -1, count);
	return;
}

void disk_populate(struct disk_struct *disk, FILE *stream)
{
	int ch;
	int file_id = 0;
	bool append_file = true;
	while((ch = fgetc(stream)) != EOF) {
		if (ch == '\n')
			continue;
		int blk_count = ch - '0';
		if (append_file) {
			disk_append_file(disk, file_id, blk_count);
			file_id += 1;
		} else {
			disk_append_free_blk(disk, blk_count);
		}
		append_file = !append_file;
	}
	return;
}

static struct disk_struct *new_empty_disk(void)
{
	struct disk_struct *empty_disk;
	empty_disk = malloc(sizeof*empty_disk);
	assert(empty_disk != NULL);
	empty_disk->blk = NULL;
	empty_disk->blk_count = 0;
	empty_disk->capacity = 0;
	return empty_disk;
}

static int disk_last_file_block(struct disk_struct *disk, size_t last_blk)
{
	int i;
	assert(last_blk <= disk->blk_count);
	for (i = (last_blk - 1); i >= 0; i -= 1) {
		if (disk->blk[i] != -1) {
			break;
		}
	}
	return i;
}

static int disk_next_free_block(struct disk_struct *disk, size_t blk)
{
	size_t i;
	for (i = blk; i < disk->blk_count; i++) {
		if (disk->blk[i] == -1) {
			break;
		}
	}
	return i;
}

static void disk_move_file(struct disk_struct *disk, int file_blk,
		int free_blk)
{
	assert(disk->blk[file_blk] != -1);
	assert(disk->blk[free_blk] == -1);
	int file_id = disk->blk[file_blk];
	disk->blk[file_blk] = -1;
	disk->blk[free_blk] = file_id;
	return;
}

static void disk_compact(struct disk_struct *disk)
{
	int last_file_blk = disk->blk_count;
	int next_free_blk = 0;
	for (;;) {
		last_file_blk = disk_last_file_block(disk, last_file_blk);
		if (last_file_blk == -1)
			break;

		next_free_blk = disk_next_free_block(disk, next_free_blk);
		if (next_free_blk == -1)
			break;

		if (next_free_blk > last_file_blk)
			break;

		disk_move_file(disk, last_file_blk, next_free_blk);
	}
	return;
}

static unsigned long long disk_checksum(struct disk_struct *disk)
{
	size_t i;
	unsigned long long checksum = 0;
	for(i = 0; i < disk->blk_count; i++) {
		if (disk->blk[i] == -1)
			continue;
		checksum = checksum + (disk->blk[i] * i);
	}
	return checksum;
}

static struct disk_struct *new_disk_from_input(char *pathname)
{
	FILE *input;
	struct disk_struct *disk;
	input = fopen(pathname, "r");
	assert(input != NULL);
	disk = new_empty_disk();
	disk_populate(disk, input);
	fclose(input);
	return disk;
}

static void disk_destroy(struct disk_struct *disk)
{
	free(disk->blk);
	free(disk);
	return;
}

int main(void)
{
	struct disk_struct *disk;
	unsigned long checksum;
	disk = new_disk_from_input("input");
	disk_compact(disk);
	checksum = disk_checksum(disk);
	disk_destroy(disk);
	printf("checksum is : %lu\n", checksum);
	return 0;
}

