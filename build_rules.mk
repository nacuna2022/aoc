TOPDIR=$(patsubst %/, %, $(dir $(CURDIR)))
TOPDIR:=$(patsubst %/, %, $(dir $(TOPDIR)))

CC=gcc

CFLAGS:=-Wall -O0 -ggdb -I$(TOPDIR)/include

AOCLIBDIR:=$(TOPDIR)/libs

lib_srcs=$(AOCLIBDIR)/aoc_map.c

aoc_libs=$(patsubst %.c, %.o, $(lib_srcs))

all: $(p1) $(p2)

$(p1) : $(aoc_libs) $(p1).o
	@echo "ELF $@"
	@$(CC) $(CFLAGS) -o $@ $^

$(p2) : $(aoc_libs) $(p2).o
	@echo "ELF $@"
	@$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	@echo "CC $@"
	@$(CC) $(CFLAGS) -MMD -c -o $@ $<

$(AOCLIBDIR)/%.o: $(AOCLIBDIR)/%.c
	@echo "LIB $@"
	@$(CC) $(CFLAGS) -MMD -c -o $@ $<

LIBS_DEP=$(patsubst %.c, %.d, $(lib_srcs))

.PHONY: clean tags
clean:
	rm -rf $(p1) $(p2)
	rm -rf *.o
	rm -rf *.d
	rm -rf $(aoc_libs)
	rm -rf $(LIBS_DEP)

tags:
	find . -name "*.[ch]" -exec ctags --append {} +

-include $(patsubst %, %.d, $(p1) $(p2))
-include $(LIBS_DEP)

