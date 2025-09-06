// ostep-projects/initial-utilities/wgrep/wgrep.c
// Created on: Fri Sep  5 23:55:16 +01 2025

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void grepper(FILE *stream, const char *searchterm) {
  char *line = NULL;
  size_t len = 0;

  while (getline(&line, &len, stream) != -1) {
    if (strstr(line, searchterm) != NULL) {
      printf("%s", line);
    }
  }

  free(line);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("wgrep: searchterm [file ...]\n");
    exit(EXIT_FAILURE);
  }

  const char *searchterm = argv[1];

  if (argc == 2) {
    grepper(stdin, searchterm);
  } else {
    for (size_t i = 2; i < argc; i++) {
      FILE *fp = fopen(argv[i], "r");
      if (fp == NULL) {
        printf("wgrep: cannot open file\n");
        exit(EXIT_FAILURE);
      }
      grepper(fp, searchterm);
      fclose(fp);
    }
  }

  return EXIT_SUCCESS;
}
