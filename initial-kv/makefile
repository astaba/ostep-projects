# ostep-projects/initial-kv/makefile
# Created on: Wed Sep 10 05:55:15 +01 2025

.PHONY : all clean test
.DELETE_ON_ERROR:

CC       := gcc
CFLAGS   := -Wall -Werror
DBGFLAGS := -g3 -O0 -DDEBUG
# SRCS     := kv.c
SRCS     := kv-v1.c
# MYSRCS   := kv-v1.c kv.c
# MYBINS   := $(subst .c,.out,$(MYSRCS))

all: kv $(MYBINS)

kv: $(SRCS)
	$(CC) $(CFLAGS) $(DBGFLAGS) $< -o $@

# $(MYBINS): %.out: %.c
# 	$(CC) $(CFLAGS) $(DBGFLAGS) $< -o $@

test: kv
	./test-kv.sh

clean:
	rm -fv *.out kv
	rm -rf ./tests-out
