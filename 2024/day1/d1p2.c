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

struct avl_node {
	int n;
	int height;
	int count;
	struct avl_node *left;
	struct avl_node *right;
};

static int *list1 = NULL;
static int list1_count = 0;
static int list1_size = 0;
static const int growth_factor = 64;

/* our majestic AVL binary search tree */
static struct avl_node *root = NULL;

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

static void append_list1(int num)
{
	list1[list1_count] = num;
	list1_count += 1;
	return;
}

static void grow_list1(void)
{
	int new_size = list1_size + growth_factor;
	list1 = realloc(list1, sizeof(int) * new_size);
	if (!list1) {
		die(-1, "no memory\n");
	}
	list1_size = new_size;
	return;
}

/* implicitly works on list1 */
static void append_int(int num)
{
	if (list1_count == list1_size) {
		grow_list1();
	}

	append_list1(num);

	return;
}

static struct avl_node *avl_node_new_from_int(int num)
{
	struct avl_node *node;
	node = malloc(sizeof *node);
	if (!node) {
		die(-1, "no memory\n");
	}
	node->n = num;
	node->height = 1;
	node->count = 1;
	node->left = NULL;
	node->right = NULL;
	return node;
}

static int avl_node_height(struct avl_node *node)
{
	if (node)
		return node->height;
	return 0;
}

/* when adjusting the root node's height, we simply get the taller between
 * its children (if they exist) and add 1.
 */
static void adjust_root_height(struct avl_node *root)
{
	struct avl_node *left_child = root->left;
	struct avl_node *right_child = root->right;
	int left_height;
	int right_height;

	left_height = avl_node_height(left_child);
	right_height = avl_node_height(right_child);

	if (left_height > right_height) {
		root->height = left_height + 1;
	} else if (right_height > left_height) {
		root->height = right_height + 1;
	} else { /* they are equal so it doesn't really matter 
		  * if left or right. let's just use right child */
		root->height = right_height + 1;
	}

	return;
}

static int balance_factor(struct avl_node *root)
{
	struct avl_node *left_child = root->left;
	struct avl_node *right_child = root->right;
	int left_height;
	int right_height;
	left_height = avl_node_height(left_child);
	right_height = avl_node_height(right_child);
	return left_height - right_height;
}

static void ll_rotate(struct avl_node **root)
{
	struct avl_node *new_root;

	new_root = (*root)->left;
	(*root)->left = new_root->right;
	new_root->right = *root;

	adjust_root_height(*root);
	adjust_root_height(new_root);

	*root = new_root;
	return;
}

static void lr_rotate(struct avl_node **root)
{
	struct avl_node *new_root;

	new_root = (*root)->left->right;

	(*root)->left->right = new_root->left;
	new_root->left = (*root)->left;

	(*root)->left = new_root->right;
	new_root->right = *root;

	*root = new_root;
	adjust_root_height((*root)->left);
	adjust_root_height((*root)->right);
	adjust_root_height(*root);

	return;
}

static void rr_rotate(struct avl_node **root)
{
	struct avl_node *new_root;

	new_root = (*root)->right;
	(*root)->right = new_root->left;
	new_root->left = *root;

	adjust_root_height(*root);
	adjust_root_height(new_root);
	
	*root = new_root;
	return;
}

static void rl_rotate(struct avl_node **root)
{
	struct avl_node *new_root;

	new_root = (*root)->right->left;

	(*root)->right->left = new_root->right;
	new_root->right = (*root)->right;

	(*root)->right = new_root->left;
	new_root->left = *root;

	*root = new_root;
	adjust_root_height((*root)->left);
	adjust_root_height((*root)->right);
	adjust_root_height(*root);

	return;
}

static void avl_insert(struct avl_node **root, int n)
{
	struct avl_node *node;
	if (*root == NULL) {
		node = avl_node_new_from_int(n);
		*root = node;
		return;
	}

	if (n < (*root)->n) {
		avl_insert(&(*root)->left, n);
	} else {
		avl_insert(&(*root)->right, n);
	}

	adjust_root_height(*root);

	if ((balance_factor(*root) == 2) && (balance_factor((*root)->left) == 1)) {
		ll_rotate(root);
	} else if  ((balance_factor(*root) == 2) && (balance_factor((*root)->left) == -1)) {
		lr_rotate(root);
	} else if ((balance_factor(*root) == -2) && (balance_factor((*root)->right) == -1)) {
		rr_rotate(root);
	} else if ((balance_factor(*root) == -2) && balance_factor((*root)->right) == 1) {
		rl_rotate(root);
	}
	return;
}

/* perform in-order traversal */
static struct avl_node *avl_find(struct avl_node *root, int n)
{
	struct avl_node *node;

	if (root == NULL)
		return NULL;

	if ((node = avl_find(root->left, n)) != NULL) {
		return node;
	}

	if (root->n == n) {
		return root;
	}

	if ((node = avl_find(root->right, n)) !=NULL) {
		return node;
	}

	return NULL;
}

/* we perform _fast_ delete of AVL tree without doing the rotations anymore 
 * since we are only focused on cleanup of _entire_ tree. */
static void avl_delete_tree(struct avl_node **root)
{
	if (*root == NULL) {
		return;
	}

	while ((*root)->left != NULL) {
		avl_delete_tree(&(*root)->left);
	}

	while ((*root)->right != NULL) {
		avl_delete_tree(&(*root)->right);
	}

	free(*root);
	*root = NULL;

	return;
}

int main(void)
{
	int fd;
	int i;
	int similarity_score;
	
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

	if ((list1 = malloc(sizeof(int) * growth_factor)) == NULL) {
		die(-1, "no memory\n");
	}

	tk = '\n';
	while(tk) {
		struct avl_node *node;

		/* first token is an int literal */
		next();
		if (tk == 0)
			break;

		append_int(tk_value);

		next();
		while(tk != TOKEN_INT_LITERAL) {
			next();
		}

		/* second token is an int literal as well */
		next();
		if ((node = avl_find(root, tk_value)) != NULL) {
			node->count+=1;
			continue;
		}
		avl_insert(&root, tk_value);
	}

	similarity_score = 0;
	for (i=0; i<list1_count; i++) {
		struct avl_node *node;
		int tmp = 0;
		node = avl_find(root, list1[i]);
		if (node) {
			tmp = node->count;
		}
		similarity_score = similarity_score + (list1[i] * tmp);
	}
	printf("similarity score is %d\n" , similarity_score);
	
	avl_delete_tree(&root);
	free(list1);
	munmap(map, st.st_size);
	close(fd);
	return 0;
}

