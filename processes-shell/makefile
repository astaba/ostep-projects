# ostep-projects/processes-shell/makefile
# Created on: Thu Sep 18 16:05:04 +01 2025

.PHONY: all clean test
.DELETE_ON_ERROR:

# ==============================================================================
CC       := gcc
CFLAGS   := -Wall -Werror
# Optional debug flags for development
DBGFLAGS := -g3 -O0 -DDEBUG
# ==============================================================================
# HACK: Add more sources and use the $(word <no>,$(...)) syntax by
# changing the <no> on the MAINTARGET recipe to build from different sources
SRCS       := wish.c
MAINTARGET := $(subst .c,,$(word 1,$(SRCS)))
ELFS       := $(subst .c,.out,$(filter-out $(word 1,$(SRCS)),$(SRCS)))
# ==============================================================================

all: $(MAINTARGET) $(ELFS)

# ==============================================================================
$(MAINTARGET): $(word 1,$(SRCS))
	$(CC) $(CFLAGS) $(DBGFLAGS) $< -o $@

$(ELFS): %.out: %.c
	$(CC) $(CFLAGS) $(DBGFLAGS) $< -o $@

# ==============================================================================
test: $(MAINTARGET)
	./test-wish.sh

# ==============================================================================
clean:
	rm -fv $(MAINTARGET) *.out
	rm -rf ./tests-out
