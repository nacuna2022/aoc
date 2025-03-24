#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdbool.h>

enum {
        TOKEN_INT_LITERAL = 128, /* we only have 1 type of token for now */
};

static char *map;

static int tk; /* token */
static int tk_value; /* token value */
static char *p; /* character iterator */

struct node {
        int n;
        struct node *prev;
        struct node *next;
} reports = {.prev = &reports, .next = &reports}; 

static void die(int status, char *fmt, ...)
{
        va_list va;
        va_start(va, fmt);
        vfprintf(stderr, fmt, va);
        va_end(va);
        exit(status);
}

static void next(void)
{
        while((tk = *p) != 0) {
                p++;
                if (tk >= '0' && tk <= '9') {
                        tk_value = tk - '0';
                        for (; *p >= '0' && *p <= '9'; p++) {
                                tk_value = (tk_value*10) + (*p - '0');
                        }
                        tk = TOKEN_INT_LITERAL;
                        return;
                }
                switch(tk) {
                case '\n':
                        return;
                }
        }
        return;
}

static struct node *new_node_from_int(int num)
{
        struct node *node;
        if ((node = malloc(sizeof *node))) {
                node->n = num;
                node->prev = node->next = NULL;
        }
        return node;
}

static void enqueue_node(struct node *list, struct node *node)
{
        struct node *next = list;
	struct node *prev = list->prev;

	prev->next = node;
	node->prev = prev;

	next->prev = node;
	node->next = next;
	
        return;
}

static void enqueue_int(struct node *list, int num)
{
        struct node *node;
        node = new_node_from_int(num);
        if (!node) {
                die(-1, "no memory\n");
        }
        enqueue_node(list, node);
        return;
}

static void free_list(struct node *list)
{
        struct node *pnode = list->next;
        while(pnode != list) {
                struct node *prev = pnode->prev;
                struct node *next = pnode->next;
                struct node *tmp = pnode;

                prev->next = pnode->next;
                next->prev = pnode->prev;
                pnode = pnode->next;
                free(tmp);
        }
        return;
}

static bool report_is_increasing(struct node *list)
{
	struct node *first = list->next;
	struct node *last = list->prev;
	struct node *tmp = first;

	bool increasing = first->n < last->n;
	if (!increasing)
		return false;

	int n = tmp->n;
	for (tmp = tmp->next; tmp != list; tmp = tmp->next) {
		if ((tmp->n <= n) || (abs(tmp->n - n) > 3)) {
			return false;
		}
		n = tmp->n;
	}
	return true;
}

static bool report_is_decreasing(struct node *list)
{
	struct node *first = list->next;
	struct node *last = list->prev;
	struct node *tmp = first;

	bool decreasing = first->n > last->n;
	if (!decreasing)
		return false;

	int n = tmp->n;
	for (tmp = tmp->next; tmp != list; tmp = tmp->next) {
		if ((tmp->n >= n) || (abs(n - tmp->n) > 3)) {
			return false;
		}
		n = tmp->n;
	}
	return true;
}

static bool report_list_is_safe(struct node *list)
{
	if (report_is_increasing(list)) {
		return true;
	}
	if (report_is_decreasing(list)) {
		return true;
	}
	return false;
}

int main(void)
{
        int fd;
        struct stat st;
	int safe_count;
        if ((fd = open("input", O_RDONLY)) == -1) {
                die(-1, "input file not found\n");
        }
        fstat(fd, &st);
        if ((map = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED) {
                close(fd);
                die(-2, "cannot map input file\n");
        }
        p = map;

        tk = '\n';
	safe_count = 0;
        while(tk) {
                next();
                if (tk == 0)
                        break;

		if (tk == '\n') {
			if (report_list_is_safe(&reports)) {
				safe_count += 1;
			}
			free_list(&reports);
			reports.prev = reports.next = &reports;
			continue;
		}

		enqueue_int(&reports, tk_value);
        }

	printf("%d reports are safe.\n", safe_count);
        munmap(map, st.st_size);
        close(fd);
        return 0;
}

