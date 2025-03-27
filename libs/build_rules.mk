
lib_srcs-y=

lib_srcs-$(CONFIG_INPUT_CACHE)+=libs/input_cache.c
lib_srcs-$(CONFIG_LINE_CACHE)+=libs/line_cache.c
lib_srcs-$(CONFIG_MAP_CACHE)+=libs/map.c
lib_srcs-$(CONFIG_DIE)+=libs/die.c

aoc_libs=$(patsubst libs/%.c, %.o, $(lib_srcs-y))

