.PHONY : all clean
.DELETE_ON_ERROR:

CC     := gcc
CFLAGS := -Wall -Werror

all: wcat ycat.out

wcat: wcat.c
	$(CC) $(CFLAGS)  $< -o $@

ycat.out: ycat.c
	$(CC) $(CFLAGS)  $< -o $@

clean:
	rm -fv *.out wcat
	rm -rf ./tests-out
