// ostep-projects/initial-utilities/wgrep/ygrep.c
// Created on: Sat Sep  6 04:10:29 +01 2025

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void searchfile(FILE *stream, const char *searchterm, char *filename) {
  char *line = NULL;
  size_t len = 0;
  size_t lineno = 0;
  size_t colmno;
  size_t searchlen = strlen(searchterm);

  while (getline(&line, &len, stream) != -1) {
    lineno++;
    char *p = line;

    while ((p = strstr(p, searchterm)) != NULL) {
      colmno = p - line + 1; // Get 1-based column of the match
      printf("%s:%zu:%zu:%s", filename, lineno, colmno, line);
      // If the line doesn’t end in \n (like last line of file), output
      // formatting might be odd (no newline). A safe bet:
      if (line[strlen(line) - 1] != '\n')
        putchar('\n');
      // Advance p beyond the match for next strstr() call
      p += searchlen;
    }
  }

  free(line);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "ygrep: %s <searchterm> [file ...]\n", argv[0]);
    exit(2);
  }

  const char *searchterm = argv[1];

  if (argc == 2) {
    searchfile(stdin, searchterm, "stdin");
  } else {
    for (size_t i = 2; i < argc; i++) {
      char *filename = argv[i];
      FILE *fp = fopen(filename, "r");
      if (fp == NULL) {
        fprintf(stderr, "ygrep: cannot open file %s: %s\n", filename,
                strerror(errno));
        exit(EXIT_FAILURE);
      }
      searchfile(fp, searchterm, filename);
      fclose(fp);
    }
  }

  return EXIT_SUCCESS;
}
