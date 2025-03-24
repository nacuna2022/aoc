#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <assert.h>

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
} list1 = {.prev = &list1, .next = &list1}, 
  list2 = {.prev = &list2, .next = &list2};

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

static void insert_node_sorted(struct node *list, struct node *node)
{
        struct node *pnode = list->next;

        /* list is empty, forego comparison */
        if (pnode == list) {
                pnode->prev = node;
                pnode->next = node;
                node->next = pnode;
                node->prev = pnode;
                return;
        }

        for (; pnode != list; pnode = pnode->next) {
                if (pnode->n > node->n) {
                        break;
                }
        }

        node->next = pnode;
        node->prev = pnode->prev;
        pnode->prev = node;
        node->prev->next = node;
        return;
}

static void insert_list_sorted(struct node *list, int num)
{
        struct node *node;
        node = new_node_from_int(num);
        if (!node) {
                die(-1, "no memory\n");
        }
        insert_node_sorted(list, node);
        return;
}

/* assumption is that list1 and list2 have equal nodes */
static int find_list_distance(struct node *list1, struct node *list2)
{
        int distance = 0;
        struct node *rnode = list1->next;
        struct node *lnode = list2->next;
        while(rnode != list1 && lnode != list2) {
                distance = distance + abs(rnode->n - lnode->n);
                rnode = rnode->next;
                lnode = lnode->next;
        }
        return distance;
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

int main(void)
{
        int fd;
        struct stat st;
        if ((fd = open("input", O_RDONLY)) == -1) {
                die(-1, "input file not found\n");
        }
        fstat(fd, &st);
        if ((map = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED) {
                close(fd);
                die(-2, "cannot map input file\n");
        }
        p = map;

        /* we can iterate the file line by line and sscanf() each line with a
         * format string of "%d %d". also, utilizing mmap() and other stuff 
         * will tie this code to linux only. but whatever. */

        tk = '\n';
        while(tk) {
                /* first token is an int literal */
                next();
                if (tk == 0)
                        break;

                insert_list_sorted(&list1, tk_value);

                next();
                while(tk != TOKEN_INT_LITERAL) {
                        next();
                }

                /* second token is an int literal as well */
                next();
                insert_list_sorted(&list2, tk_value);
        }

        printf("distance is %d\n", find_list_distance(&list1, &list2));
        free_list(&list1);
        free_list(&list2);
        munmap(map, st.st_size);
        close(fd);
        return 0;
}

