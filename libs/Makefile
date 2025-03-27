include ../.config
include build_rules.mk

libaoc.a: $(aoc_libs)
	ar rcs $@ $^

%.o: %.c
	@echo "LIB $@"
	@$(CC) $(CFLAGS) -I$(TOPDIR)/include -MMD -c -o $@ $<

LIBS_DEP=$(patsubst %.o, %.d, $(aoc_libs))

clean:
	@rm -rf *.a
	@rm -rf *.o
	@rm -rf *.d

-include $(LIBS_DEP)
