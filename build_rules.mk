.DEFAULT_GOAL:=all

all: $(p1) $(p2) 

$(p1) : $(aoc_libs) $(p1).o ../../libs/libaoc.a
	@echo "ELF $@"
	@$(CC) $(CFLAGS) -o $@ $^ -laoc

$(p2) : $(aoc_libs) $(p2).o ../../libs/libaoc.a
	@echo "ELF $@"
	@$(CC) $(CFLAGS) -o $@ $^ -laoc

%.o: %.c
	@echo "CC $@"
	@$(CC) $(CFLAGS) -MMD -c -o $@ $<

.PHONY: clean tags
clean:
	@rm -rf $(p1) $(p2)
	@rm -rf *.o
	@rm -rf *.d

tags:
	find . -name "*.[ch]" -exec ctags --append {} +

-include $(patsubst %, %.d, $(p1) $(p2))

