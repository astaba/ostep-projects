// ostep-projects/initial-utilities/wcat/ycat.c
// Create on: Fri Sep  5 21:05:16 +01 2025
// INFO:
// My primitive emulator of "cat" utility
// Improvement:
// 1. On invalid argument exit with POSIX standard explicit code
// 2. On fopen() failure (non-existent) print more explicit error message
//    but do not exit. Save exit status and proceed to next arguments.
// 3. Performance: IO with fread/fwrite is much faster than fgets/printf

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef BUFSIZ
#define BUFSIZ 4096
#endif
#define EXIT_USAGE_ERROR 2

int main(int argc, char *argv[]) {
  int exit_status = EXIT_SUCCESS;

  if (argc == 1) {
    /* return EXIT_SUCCESS; // Test 4 */

    // POSIX conventions: usage error (e.g., invalid arguments)
    return EXIT_USAGE_ERROR;
  }

  char *buffer = malloc(BUFSIZ);
  if (buffer == NULL) {
    perror("ycat: buffer allocation failed");
    exit_status = EXIT_FAILURE;
    return exit_status;
  }

  // Process each file argument in order
  for (size_t i = 1; i < argc; i++) {

    FILE *fp = fopen(argv[i], "r");
    if (fp == NULL) {
      /* printf("wcat: cannot open file\n"); // Test 6
      exit(EXIT_FAILURE); */

      // My version: Do not terminate on non-existent file.
      // Print explicit error message and proceed to next argument.
      fprintf(stderr, "ycat: cannot open file %s: %s\n", argv[i],
              strerror(errno));
      exit_status = EXIT_FAILURE;
      continue;
    }
    // read stream file chunk by chunk and write to stdout
    size_t bytes_read = 0;
    while ((bytes_read = fread(buffer, sizeof(char), BUFSIZ, fp)) > 0) {
      if (fwrite(buffer, sizeof(char), bytes_read, stdout) < bytes_read) {
        fprintf(stderr, "ycat: error writing file %s: %s\n", argv[i],
                strerror(errno));
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
