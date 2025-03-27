#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <aoc/die.h>

void aoc_die(int status, char *fmt, ...)
{
        va_list va;
        va_start(va, fmt);
        vfprintf(stderr, fmt, va);
        va_end(va);
        exit(status);
}

