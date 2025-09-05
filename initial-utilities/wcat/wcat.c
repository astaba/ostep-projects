// Create on: Fri Sep  5 19:46:25 +01 2025

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef BUFSIZ
#define BUFSIZ 4096
#endif

int main(int argc, char *argv[]) {
  if (argc == 1) {
    exit(EXIT_SUCCESS);
  }

  char *buffer = malloc(sizeof(char) * BUFSIZ);
  if (buffer == NULL) {
    perror("wcat: buffer allocation failed");
    exit(EXIT_FAILURE);
  }

  for (size_t i = 1; i < argc; i++) {
    FILE *fp = fopen(argv[i], "r");
    if (fp == NULL) {
      printf("wcat: cannot open file\n");
      exit(EXIT_FAILURE);
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
      printf("%s", buffer);
    }
    if (ferror(fp)) {
      fprintf(stderr, "wcat: error reading file %s: %s", argv[i],
              strerror(errno));
      fclose(fp);
      exit(EXIT_FAILURE);
    }
    fclose(fp);
  }

  free(buffer);
  return EXIT_SUCCESS;
}
