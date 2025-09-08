# ostep-projects/initial-reverse/makefile
# Created on: Mon Sep  8 18:26:30 +01 2025

.PHONY : all clean test
.DELETE_ON_ERROR:

CC       := gcc
CFLAGS   := -Wall -Werror
# DBGFLAGS := -g3 -O0 -DDEBUG
SRCS     := reverse.c
MYSRCS   :=
MYBINS   := $(subst .c,.out,$(MYSRCS))

all: reverse $(MYBINS)

reverse: $(SRCS)
	$(CC) $(CFLAGS) $(DBGFLAGS) $< -o $@

$(MYBINS): %.out: %.c
	$(CC) $(CFLAGS) $(DBGFLAGS) $< -o $@

test: reverse
	./test-reverse.sh

clean:
	rm -fv *.out reverse
	rm -rf ./tests-out
