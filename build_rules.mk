CC=gcc

all: $(p1)

$(p1) : $(p1).o $(p2).o
	@echo "ELF $@"
	@$(CC) -o $@ $<

%.o: %.c
	@echo "CC $@"
	@$(CC) -MMD -O0 -ggdb -c -o $@ $<

.PHONY: clean tags
clean:
	rm -rf $(p1) $(p2)
	rm -rf *.o
	rm -rf *.d

tags:
	find . -name "*.[ch]" -exec ctags --append {} +

-include $(patsubst %, %.d, $(p1) $(p2))

