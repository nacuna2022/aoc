#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#define MAX_LINE_SIZE	1024

struct eq_struct {
#define NUM_COUNT	4
	unsigned long long test_value; /* appears before the colon */
	size_t size;
	size_t count;
	unsigned long long *numbers;
	struct eq_struct *next; /* singly */
};

static struct eq_struct *list_head;
static struct eq_struct *list_tail;

static void eq_append_num(struct eq_struct *eq, int num)
{
	unsigned long long *new_numbers;
	size_t new_size;
	while (eq->count >= eq->size) {
		new_size = eq->size + NUM_COUNT;
		new_numbers = realloc(eq->numbers, sizeof(unsigned long long) * new_size);
		assert(new_numbers != NULL);
		eq->numbers = new_numbers;
		eq->size = new_size;
	}
	eq->numbers[eq->count] = num;
	eq->count += 1;
	return;
}

static struct eq_struct *eq_init(struct eq_struct *eq, char *line)
{
	unsigned long long num;
	char *next;
	num = strtoull(line, &next, 10);
	assert(*next == ':');
	eq->test_value = (unsigned long long)num;
	eq->size = 0;
	eq->count = 0;
	eq->numbers = NULL;
	
	next += 1;
	num = strtol(next, &next, 10);
	while(*next != '\0') {
		eq_append_num(eq, num);
		next += 1;
		num = strtol(next, &next, 10);
	}
	return eq;
}

static struct eq_struct *alloc_eq(char *line)
{
	struct eq_struct *eq;
	if ((eq = malloc(sizeof * eq)) == NULL) {
		fprintf(stderr, "error allocating memory for eq object\n");
		exit(-1);
	}
	return eq_init(eq, line);
}


#if 0
we need to test all the possible combinations of + or * for a list of numbers
and check if any one of those results into the test_value. 

For example: if we this eq line -> 292: 11 6 16 20.

             +20 => 53
             |
        +16 -|
        |    |
	|    *20 => 660
    +6 -|    
    |   |    +20 => 292 (OK!)
    |   |    |
    |   *16 -|
    |        |
    |        *20 => 5440
11 -|        
    |        +20 => 102
    |        |  
    |   +16 -|
    |   |    |
    |   |    *20 => 1640
    *6 -|
        |    +20 => 1076
	|    |
        *16 -|
	     |
	     *20 => 21120
#endif

static bool test_eq(struct eq_struct *eq, unsigned long long num, size_t idx)
{
	if (idx < (eq->count-1)) {
		idx += 1;

		/* check the + path first */
		if (test_eq(eq, num + eq->numbers[idx], idx) == false) {

			/* if it fails, check the * path */
			return test_eq(eq, num * eq->numbers[idx], idx);
		}
		return true;
	}
	return num == eq->test_value;
}

static bool eq_ok(struct eq_struct *eq)
{
	assert(eq->count > 1); /* we have at least two numbers in eq */
	return test_eq(eq, eq->numbers[0], 0);
}

static void clean_up(void)
{
	struct eq_struct *eq;
	while (list_head) {
		eq = list_head->next;
		free(list_head);
		list_head = eq;
	}
	return;
}

int main(void)
{
	char line_buf[MAX_LINE_SIZE];
	FILE *input;
	struct eq_struct *eq;
	unsigned long long total_calibration = 0;

	if ((input = fopen("input", "r")) == NULL) {
		fprintf(stderr, "cannot open input file\n");
		exit(-1);
	}

	list_head = list_tail = NULL;
	while(fgets(line_buf, sizeof line_buf, input) != NULL) {
		eq = alloc_eq(line_buf);
		
		/* append to list */
		if (!list_head) {
			assert(!list_tail);
			list_head = list_tail = eq;
		} else {
			assert(list_tail && !list_tail->next);
			list_tail->next = eq;
			list_tail = eq;
		}
		
	}
	fclose(input);

	for (eq = list_head; eq != NULL; eq = eq->next) {
		total_calibration += (eq_ok(eq) == true) ? eq->test_value: 0;
	}
	printf("total calibration result: %llu\n", total_calibration);
	clean_up();
	return 0;
}

