// Create on: Fri Sep  5 21:05:16 +01 2025
// Primitiv emulator of "cat" utility

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef BUFSIZ
#define BUFSIZ 4096
#endif

int main(int argc, char *argv[]) {
  int exit_status = EXIT_SUCCESS;

  if (argc == 1)
    return 2; // POSIX conventions: usage error (e.g., invalid arguments)

  // Allocate a reusable buffer for file I\O
  char *buffer = malloc(sizeof(char) * BUFSIZ);
  if (buffer == NULL) {
    perror("ycat: buffer allocation failed");
    exit(EXIT_FAILURE);
  }

  // Process each file argument in order
  for (size_t i = 1; i < argc; i++) {

    FILE *fp = fopen(argv[i], "r");
    if (fp == NULL) {
      perror(argv[i]);
      exit_status = EXIT_FAILURE;
      continue;
    }
    // read stream file chunk by chunk and write to stdout
    size_t bytes_read = 0;
    while ((bytes_read = fread(buffer, sizeof(char), BUFSIZ, fp)) > 0) {
      if (fwrite(buffer, sizeof(char), bytes_read, stdout) < bytes_read) {
        perror("ycat: write error");
        exit_status = EXIT_FAILURE;
        break;
      }
    }
    // if fread stopped because of error (not just EOF)
    if (ferror(fp)) {
      fprintf(stderr, "ycat: error reading file %s: %s\n", argv[i],
              strerror(errno));
      exit_status = EXIT_FAILURE;
      fclose(fp);
      continue;
    }

    fclose(fp);
    // HACK: In production-grade coreutil replacements check all fclose():
    // if (fclose(fp) == EOF) {
    //   fprintf(stderr, "ycat: error closing file %s: %s\n", argv[i],
    //           strerror(errno));
    //   exit_status = EXIT_FAILURE;
    // }
  }

  free(buffer);
  return exit_status;
}
