lib_srcs=

ifeq ($(aoc_map),y)
lib_srcs+=$(AOCLIBDIR)/aoc_map.c
endif

aoc_libs=$(patsubst %.c, %.o, $(lib_srcs))

$(AOCLIBDIR)/%.o: $(AOCLIBDIR)/%.c
	@echo "LIB $@"
	@$(CC) $(CFLAGS) -MMD -c -o $@ $<

LIBS_DEP=$(patsubst %.c, %.d, $(lib_srcs))

.PHONY: clean_lib
clean_lib:
	rm -rf $(aoc_libs)
	rm -rf $(LIBS_DEP)

-include $(LIBS_DEP)
