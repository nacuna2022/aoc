
lib_srcs-y=

lib_srcs-$(CONFIG_INPUT_CACHE)+=libs/incache.c
lib_srcs-$(CONFIG_LINE_CACHE)+=libs/lcache.c
lib_srcs-$(CONFIG_MAP_CACHE)+=libs/mcache.c

# die is always present
lib_srcs-y+=libs/die.c

aoc_libs=$(patsubst libs/%.c, %.o, $(lib_srcs-y))

