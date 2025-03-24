#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <string.h>

enum {
        TOKEN_INT_LITERAL = 128,
	TOKEN_MULT_INST, /* multiply token */
	TOKEN_DONT_INST, /* don't instruction */
	TOKEN_DO_INST,	 /* do instruction */
};

static char *map;

static int tk; /* token */
static int tk_value; /* token value */
static char *p; /* character iterator */

static void die(int status, char *fmt, ...)
{
        va_list va;
        va_start(va, fmt);
        vfprintf(stderr, fmt, va);
        va_end(va);
        exit(status);
}

/* there is no rigid grammar rule to follow here except a simple check for
 * "mul" operator followed by open parens '(' then an integer, then a comma ','
 * then second integer, and lastly a close parens ')'.
 *
 * e.g. mul(int,int)
 */
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
		case 'm':
			p--;
			if (memcmp(p, "mul", 3) == 0) {
				p += 3;
				tk = TOKEN_MULT_INST;
			} else {
				p++;
			}
			return;
		case 'd':
			p--;
			if (memcmp(p, "don't", 5) == 0) {
				p += 5;
				tk = TOKEN_DONT_INST;
			} else if (memcmp(p, "do", 2) == 0) {
				p += 2;
				tk = TOKEN_DO_INST;
			} else {
				p++;
			}
			return;
		case ' ':
		case '(':
		case ',':
		case ')':
		case '\n':
		default:
			return;
		}
	}
        return;
}

int main(void)
{
        int fd;
        struct stat st;
	int sum;
	bool enable = true;
        if ((fd = open("input", O_RDWR)) == -1) {
                die(-1, "input file not found\n");
        }
        fstat(fd, &st);
        if ((map = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
                close(fd);
                die(-2, "cannot map input file\n");
        }
        p = map;

        tk = '\n';
	sum = 0;

	next();
        while(tk) {
                if (tk == 0)
                        break;

		/* no nested "mul(mul(), mul())" blocks allowed. e.g. terminate an active 
		 * mul block with close parens ')' before processing the next. 
		 */
		while (tk == TOKEN_MULT_INST) {
			int n1;
			int n2;

			next();
			if (tk != '(')
				continue;

			next();
			if (tk != TOKEN_INT_LITERAL)
				continue;
			n1 = tk_value;

			next();
			if (tk != ',')
				continue;

			next();
			if(tk != TOKEN_INT_LITERAL)
				continue;
			n2 = tk_value;

			next();
			if (tk != ')')
				continue;
			
			if (enable)
				sum = sum + (n1 * n2);
		}
		
		if (tk == TOKEN_DO_INST) {
			next();
			if (tk != '(')
				continue;

			next();
			if (tk != ')')
				continue;
			enable = true;
		} else if (tk == TOKEN_DONT_INST) {
			next();
			if (tk != '(')
				continue;
			next();
			if (tk != ')')
				continue;
			enable = false;
		}
		next();
        }
	printf("sum: %d\n", sum);
        munmap(map, st.st_size);
        close(fd);
        return 0;
}

