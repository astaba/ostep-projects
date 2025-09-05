# ostep-projects/initial-utilities/wcat/makefile
# Created on: Fri Sep  5 23:48:25 +01 2025

.PHONY : all clean test
.DELETE_ON_ERROR:

CC     := gcc
CFLAGS := -Wall -Werror

SRC    := wcat.c
# SRC    := ycat.c

all: wcat ycat.out

wcat: $(SRC)
	$(CC) $(CFLAGS)  $< -o $@

ycat.out: ycat.c
	$(CC) $(CFLAGS)  $< -o $@

test: wcat
	./test-wcat.sh

clean:
	rm -fv *.out wcat
	rm -rf ./tests-out
