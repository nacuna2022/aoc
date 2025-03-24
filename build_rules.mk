CC=gcc

all: $(p1) $(p2)

$(p1) : $(p1).o
	@echo "ELF $@"
	@$(CC) -Wall -o $@ $<

$(p2) : $(p2).o
	@echo "ELF $@"
	@$(CC) -Wall -o $@ $<

%.o: %.c
	@echo "CC $@"
	@$(CC) -Wall -MMD -O0 -ggdb -c -o $@ $<

.PHONY: clean tags
clean:
	rm -rf $(p1) $(p2)
	rm -rf *.o
	rm -rf *.d

tags:
	find . -name "*.[ch]" -exec ctags --append {} +

-include $(patsubst %, %.d, $(p1) $(p2))

