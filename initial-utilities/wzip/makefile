# ostep-projects/initial-utilities/wzip/makefile
# Created on: Sun Sep  7 04:57:26 +01 2025

.PHONY : all clean test
.DELETE_ON_ERROR:

CC       := gcc
CFLAGS   := -Wall -Werror
# DBGFLAGS := -g3 -O0 -DDEBUG
SRCS     := wzip.c
MYSRCS   := wzip-v0.c wzip-v1.c
MYBINS   := $(subst .c,.out,$(MYSRCS))

all: wzip $(MYBINS)

wzip: $(SRCS)
	$(CC) $(CFLAGS) $(DBGFLAGS) $< -o $@

$(MYBINS): %.out: %.c
	$(CC) $(CFLAGS) $(DBGFLAGS) $< -o $@

test: wzip
	./test-wzip.sh

clean:
	rm -fv *.out wzip
	rm -rf ./tests-out
