
ifeq ($(wildcard .config),)
.DEFAULT default:
	@kconfig-conf --alldefconfig Kconfig
endif
include .config

_TOPDIR=$(CURDIR)
_CC=gcc
_CFLAGS:=-Wall -Wextra -O0 -ggdb -I$(CURDIR) -L$(CURDIR)/libs -laoc
export TOPDIR=$(_TOPDIR)
export CFLAGS=$(_CFLAGS)
export CC=$(_CC)

include 2024/build_rules.mk

.PHONY: $(AOC_DIRS-y)

all: $(AOC_DIRS-y)

$(AOC_DIRS-y):
	@echo "--- $@ ---"
	@$(MAKE) --no-print-directory -C $@

#.PHONY: libs/libaoc.a
#libs/libaoc.a:
#	@$(MAKE) -s --no-print-directory -C libs/ libaoc.a

clean:
	@for DIR in $(AOC_DIRS-y); do \
		$(MAKE) --no-print-directory -C $$DIR clean; \
	done

lib_clean:
	@$(MAKE) --no-print-directory -C libs/ clean

menuconfig: 
	@kconfig-mconf Kconfig
