
lib_srcs-y=

lib_srcs-$(CONFIG_INPUT_CACHE)+=libs/incache.c
lib_srcs-$(CONFIG_LINE_CACHE)+=libs/lncache.c
lib_srcs-$(CONFIG_MAP_CACHE)+=libs/mapcache.c

#always present modules here
lib_srcs-y+=libs/die.c
lib_srcs-y+=libs/dlist.c

aoc_libs=$(patsubst libs/%.c, %.o, $(lib_srcs-y))

