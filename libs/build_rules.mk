
lib_srcs-y=

lib_srcs-$(CONFIG_INPUT_CACHE)+=libs/aoc_input_cache.c
lib_srcs-$(CONFIG_LINE_CACHE)+=libs/aoc_line_cache.c
lib_srcs-$(CONFIG_MAP_CACHE)+=libs/aoc_map.c
lib_srcs-$(CONFIG_DIE)+=libs/aoc_die.c

aoc_libs=$(patsubst libs/%.c, %.o, $(lib_srcs-y))

