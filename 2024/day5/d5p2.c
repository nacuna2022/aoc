#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#define ADJ_BLKSIZE     8
#define LINE_BUFSIZE    2048

/* directed graph node */
struct vertex {
        int page;
	bool visited;
        size_t adj_count;
        size_t adj_size;
        struct vertex **adj_list;
};

struct update {
	size_t count;
	size_t size;
	int *order;
};

/* assumption is 9 < page < 100 */
#define PAGE_LUT_SIZE   100
static struct vertex *page_lut[PAGE_LUT_SIZE] = {0};

static struct vertex *new_vertex_from_page(const int page)
{
        struct vertex *v;
        if ((v = malloc(sizeof*v)) != NULL) {
                v->page = page;
                v->adj_count = 0;
                v->adj_size = 0;
		v->visited = false;
                v->adj_list = NULL;
        }
        return v;
}

static struct vertex *page2vertex(const int page)
{
        struct vertex *v;
        while ((page_lut[page]) == NULL) {
                v = new_vertex_from_page(page);
                page_lut[page] = v;
        }
        return page_lut[page];
}

static void add_adjacent(struct vertex *xvertex, struct vertex *yvertex,
                const int idx)
{
        if (idx >= xvertex->adj_size) {
                size_t new_size = xvertex->adj_size + ADJ_BLKSIZE;
                xvertex->adj_list = realloc(xvertex->adj_list,
                                new_size * sizeof(struct vertex *));
                assert(xvertex->adj_list);
                xvertex->adj_size = new_size;
        }
        xvertex->adj_list[idx] = yvertex;
        xvertex->adj_count += 1;
        return;
}

static void make_adjacent(struct vertex *xvertex, struct vertex *yvertex)
{
        size_t i;

        /* just double check if yvertex is already in xvertex's 
         * adjacency list */
        for (i = 0; i<xvertex->adj_count; i++) {
                struct vertex *tmp_vertex;
                tmp_vertex = xvertex->adj_list[i];
                if (tmp_vertex->page == yvertex->page) {
                        assert(0);
                }
        }

        /* if we reach here, yvertex still not yet adjacent to xvertex
         * and we need to add it. */
        add_adjacent(xvertex, yvertex, i);
        return;
}

static void add_page_ordering_rule(const int xpage, const int ypage)
{
        assert((xpage > 9) && (ypage > 9));
        assert((xpage < 100) && (ypage < 100));
        struct vertex *xvertex = page2vertex(xpage);
        struct vertex *yvertex = page2vertex(ypage);        

        make_adjacent(xvertex, yvertex);
        return;
}

static void destroy_vertex(struct vertex *v)
{
        if (v) {
                free(v->adj_list);
                free(v);
        }
        return;
}

static void clean_page_ordering_rules(void)
{
        size_t i;
        for (i = 0; i<PAGE_LUT_SIZE; i++) {
                struct vertex *v;
                v = *(page_lut + i);
                
                destroy_vertex(v);
                page_lut[i] = NULL;
        }

        return;
}

static void line2update(char *s, struct update *update)
{
	char *st;
	assert(s);
	assert(update);
	static const size_t grow_size = 8;
	update->count = 0;
	
	st = strtok(s, ",");
	for(; st!= NULL;) {
		int tmp;
		if (update->count >= update->size) {
			size_t new_size = update->size + grow_size;
			update->order = realloc(update->order, 
					new_size * sizeof *update->order);
			update->size = new_size;
		}
		tmp = strtol(st, NULL, 10);
		update->order[update->count] = tmp;
		update->count += 1;
		st = strtok(NULL, ",");
	}
	return;
}

static void update_free(struct update *update)
{
	if(update) {
		free(update->order);
	}
	return;
}

static bool page_is_adjacent(const int xpage, const int ypage)
{
	struct vertex *xv;
	int i;
	xv = page2vertex(xpage);
	assert(xv != NULL);
	for (i = 0; i < xv->adj_count; i++) {
		struct vertex *yv;
		yv = xv->adj_list[i];
		if (yv->page == ypage) {
			/* ypage is adjacent to xpage */
			return true;
		}
	}
	return false;
}

static int process_update(struct update *update)
{
	int i;
	int j;
	for (i = 0; i < update->count; i++) {
		/* check all before */
		for (j = 0; j < i; j++) {
			if (!page_is_adjacent(update->order[j], update->order[i])) {
				return 0;
			}
		}

		/* check all after */
		for (j = i+1; j < update->count; j++) {
			if (!page_is_adjacent(update->order[i], update->order[j])) {
				return 0;
			}
		}
	}
	/* if we reach here, the print queue passes the rules */
	return update->order[update->count >> 1];
}

static void fix_update(struct update *update)
{
	int i;
	int j;
	for (i = 0; i < (update->count-1); i++) {
		for (j = i + 1; j < update->count; j++) {
			/* check if j-page is adjacent to i-page */
			if (!page_is_adjacent(update->order[i], update->order[j])) {
				int tmp = update->order[i];
				update->order[i] = update->order[j];
				update->order[j] = tmp;
			}
		}
	}
	return;
}

static int process_bad_update(struct update *update)
{
	int ans = 0;
	while((ans = process_update(update)) == 0) {
		fix_update(update);
	}
	return ans;
}

int main(void)
{
	FILE *input;
	char linebuf[LINE_BUFSIZE];
	int ans;

	if ((input = fopen("input", "r")) == NULL) {
		fprintf(stderr, "error opening input file\n");
		exit(-1);
	}

	while(fgets(linebuf, LINE_BUFSIZE, input) != NULL) {
		int xpage;
		int ypage;

		if (*linebuf == '\n')
			break;

		sscanf(linebuf, "%d|%d\n", &xpage, &ypage);
		add_page_ordering_rule(xpage, ypage);
	}

	struct update one_update = {0};
	ans = 0;
	while(fgets(linebuf, LINE_BUFSIZE, input) != NULL) {
		line2update(linebuf, &one_update);
		if (process_update(&one_update) == 0) {
			ans += process_bad_update(&one_update);
		}
	}
	update_free(&one_update);

	printf("answer is %d\n", ans);

	clean_page_ordering_rules();
	fclose(input);
	return 0;
}

