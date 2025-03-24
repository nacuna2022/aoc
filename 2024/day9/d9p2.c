#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

struct disk_object {
	int file_id;
	bool cannot_move; /* only for files */
	int size;
	struct disk_object *prev;
	struct disk_object *next;
};

struct disk_struct {
	struct disk_object contents;
};

static struct disk_object *new_disk_object(int file_id, int size)
{
	struct disk_object *object;
	object = malloc(sizeof * object);
	assert(object != NULL);
	object->file_id = file_id;
	object->cannot_move = false;
	object->size = size;
	return object;
}

static void insert_disk_object(struct disk_object *before,
		struct disk_object *new_object,
		struct disk_object *after)
{
	before->next = new_object;
	new_object->prev = before;
	new_object->next = after;
	after->prev = new_object;
	return;
}

static void append_disk_object(struct disk_object *before,
		struct disk_object *new_object)
{
	insert_disk_object(before, new_object, before->next);
	return;
}

static void disk_append_new_object(struct disk_struct *disk,
		int file_id, int size)
{
	struct disk_object *object;
	object = new_disk_object(file_id, size);
	append_disk_object(disk->contents.prev, object);
	return;
}

static void disk_append_file(struct disk_struct *disk, int file_id,
		int size)
{
	assert(file_id >= 0);
	disk_append_new_object(disk, file_id, size);
	return;
}

static void disk_append_free(struct disk_struct *disk, int size)
{
	disk_append_new_object(disk, -1, size);
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
			disk_append_free(disk, blk_count);
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
	empty_disk->contents.next = &empty_disk->contents;
	empty_disk->contents.prev = &empty_disk->contents;
	return empty_disk;
}

static struct disk_object *disk_get_last_file_object(struct disk_struct *disk)
{
	struct disk_object *object;
	for (object = disk->contents.prev;
			object != &disk->contents;
			object = object->prev) {
		if (object->file_id == -1) 
			continue;

		if (object->cannot_move == false) 
			break;
	}
	return object == &disk->contents ? NULL : object;
}

static struct disk_object *disk_get_first_free_object(struct disk_struct *disk,
		int size)
{
	struct disk_object *object;
	for (object = disk->contents.next;
			object != &disk->contents;
			object = object->next) {
		if (object->file_id != -1)
			continue;

		if (object->size >= size)
			break;
	}
	return object == &disk->contents ? NULL : object;
}

static void mark_file_object_non_movable(struct disk_object *object)
{
	assert(object->file_id != -1);
	object->cannot_move = true;
	return;
}

static struct disk_object *fragment_free_block(struct disk_object *object,
		int file_size)
{
	struct disk_object *object2;
	int object2_size;
	if (object->size == file_size) {
		return object;
	}
	assert(object->size > file_size);
	object2_size = object->size - file_size;

	object->size = file_size;
	object2 = new_disk_object(-1, object2_size);
	append_disk_object(object, object2);
	return object;
}

static void swap_disk_objects(struct disk_object *file_object,
		struct disk_object *free_object)
{
	int tmp;
	assert(file_object->file_id != -1);
	assert(free_object->file_id == -1);
	assert(file_object->size == free_object->size);
	tmp = file_object->file_id;
	file_object->file_id = free_object->file_id;
	free_object->file_id = tmp;
	return;
}

/* make sure disk_object is found in disk */
static bool disk_object_valid(struct disk_struct *disk,
		struct disk_object *object)
{
	struct disk_object *tmp;
	for (tmp = disk->contents.next;	tmp != &disk->contents;
		tmp = tmp->next) {
		if (tmp == object) {
			return true;
		}
	}
	return false;
}

static bool disk_valid_move(struct disk_struct *disk,
		struct disk_object *free_object,
		struct disk_object *file_object)
{
	bool file_object_reached = false;
	struct disk_object *tmp;

	if (!disk_object_valid(disk, free_object)) {
		return false;
	}

	if(!disk_object_valid(disk, file_object)) {
		return false;
	}

	/* make sure we can reach free_object without first reaching 
	 * the file_object first */
	for (tmp = disk->contents.next; tmp != &disk->contents;
		tmp = tmp->next) {
		if (tmp == free_object) {
			break;
		}

		if (tmp == file_object) {
			file_object_reached = true;
			break;
		}
	}
	return !file_object_reached;
}

static void disk_compact(struct disk_struct *disk)
{
	struct disk_object *file_object;
	struct disk_object *free_object;
	for (;;) {
		/* get last disk content that is a file */
		file_object = disk_get_last_file_object(disk);

		/* no more files left to move */
		if (file_object == NULL) 
			break;

		/* get first free block that is big enough for file block */
		free_object = disk_get_first_free_object(disk, 
				file_object->size);

		/* if we cant move this file i.e. no free blocks big 
		 * enough for it, mark it non-moveable. */
		if (free_object == NULL) {
			mark_file_object_non_movable(file_object);
			continue;
		}

		/* valid file moves are only possible going to the left 
		 * direction. */
		if (!disk_valid_move(disk, free_object, file_object)) {
			mark_file_object_non_movable(file_object);
			continue;
		}

		/* fragment the free block if necessary */
		free_object = fragment_free_block(free_object,
				file_object->size);

		/* file block size should be equal to file block now 
		 * so swap them */
		swap_disk_objects(file_object, free_object);
		mark_file_object_non_movable(free_object);
	}
	return;
}

static struct disk_struct *new_disk_from_input(char *pathname)
{
	FILE *input;
	struct disk_struct *disk;
	input = fopen("input", "r");
	assert(input != NULL);
	disk = new_empty_disk();
	disk_populate(disk, input);
	fclose(input);
	return disk;
}

static void disk_destroy(struct disk_struct *disk)
{
	struct disk_object *tmp;
	for (tmp = disk->contents.next; tmp != &disk->contents; ) {
		struct disk_object *next = tmp->next;
		free(tmp);
		tmp = next;
	}
	free(disk);
	return;
}

static unsigned long object_checksum(struct disk_object *object,
		int pos)
{
	unsigned long checksum = 0;
	int i;
	for (i = 0; i < object->size; i++) {
		checksum += (object->file_id * pos);
		pos += 1;
	}
	return checksum;
}

static unsigned long disk_checksum(struct disk_struct *disk)
{
	struct disk_object *tmp;
	unsigned long checksum = 0;
	int pos = 0;
	for (tmp = disk->contents.next; tmp != &disk->contents;
		tmp = tmp->next) {
		if (tmp->file_id != -1) {
			checksum = checksum + object_checksum(tmp, pos);
		}
		pos = pos + tmp->size;
	}
	return checksum;
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

