# ostep-projects/initial-utilities/wunzip/makefile
# Created on: Sun Sep  7 18:04:41 +01 2025

.PHONY : all clean test
.DELETE_ON_ERROR:

CC       := gcc
CFLAGS   := -Wall -Werror
# DBGFLAGS := -g3 -O0 -DDEBUG
SRCS     := wunzip.c
MYSRCS   :=
MYBINS   := $(subst .c,.out,$(MYSRCS))

all: wunzip $(MYBINS)

wunzip: $(SRCS)
	$(CC) $(CFLAGS) $(DBGFLAGS) $< -o $@

$(MYBINS): %.out: %.c
	$(CC) $(CFLAGS) $(DBGFLAGS) $< -o $@

test: wunzip
	./test-wunzip.sh

clean:
	rm -fv *.out wunzip
	rm -rf ./tests-out
