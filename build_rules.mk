TOPDIR=$(patsubst %/, %, $(dir $(CURDIR)))
TOPDIR:=$(patsubst %/, %, $(dir $(TOPDIR)))

CC=gcc

CFLAGS:=-Wall -O0 -ggdb -I$(TOPDIR)/include

AOCLIBDIR:=$(TOPDIR)/libs

aoc_libs=

.DEFAULT_GOAL:=all

include $(AOCLIBDIR)/lib_rules.mk

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

.PHONY: clean tags
clean:
	rm -rf $(p1) $(p2)
	rm -rf *.o
	rm -rf *.d

tags:
	find . -name "*.[ch]" -exec ctags --append {} +

-include $(patsubst %, %.d, $(p1) $(p2))

