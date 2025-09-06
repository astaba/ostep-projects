# ostep-projects/initial-utilities/wgrep/makefile
# Created on: Fri Sep  5 23:52:29 +01 2025

.PHONY : all clean test
.DELETE_ON_ERROR:

CC     := gcc
CFLAGS := -Wall -Werror
MYTAKE := ygrep-v0.c ygrep-v1.c
MYBINS := $(subst .c,.out,$(MYTAKE))

all: wgrep $(MYBINS)

wgrep: wgrep.c
	$(CC) $(CFLAGS)  $< -o $@

$(MYBINS): %.out: %.c
	$(CC) $(CFLAGS)  $< -o $@

test: wgrep
	./test-wgrep.sh

clean:
	rm -fv *.out wgrep
	rm -rf ./tests-out
