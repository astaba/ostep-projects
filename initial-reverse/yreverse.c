/* ostep-projects/initial-reverse/yreverse.c */
// Created on: Tue Sep  9 16:37:58 +01 2025

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifndef BUFSIZ
#define BUFSIZ 4096
#endif /* ifndef BUFSIZ */

typedef struct singleline {
  char *line;
  struct singleline *next;
} SINGLELINE;

int main(int argc, char *argv[]) {
  // (1) Arguments validation
  if (argc > 3) {
    fprintf(stderr, "usage: reverse <input> <output>\n");
    exit(EXIT_FAILURE);
  }

  // (2) Initialize input and output streams based on arguments numbers
  FILE *input_fp = stdin;
  FILE *output_fp = stdout;
  // Flag to prevent calling close() on standard predefined stream.
  bool is_redirected = false;

  if (argc > 1) {
    char *inpath = argv[1];
    input_fp = fopen(inpath, "r");
    if (!input_fp) {
      fprintf(stderr, "reverse: cannot open file '%s'\n", inpath);
      exit(EXIT_FAILURE);
    }

    if (argc > 2) {
      char *outpath = argv[2];
      // Make sure input and output files are not the same. --------------------
      // WARNING: By initializing `same` to `true` you tell the program:
      // "Unless I can get rock-solid proof that these files are different,
      // I'm going to assume they're the same and terminate the program."
      // Ensure Data Integrity.
      bool same = true; // Best practice although not complying to project tests
      struct stat stat1, stat2;
      errno = 0; // Allows stat() to loudly express failure.
      if (!stat(inpath, &stat1) && !stat(outpath, &stat2)) {
        same = (stat1.st_dev == stat2.st_dev) && (stat1.st_ino == stat2.st_ino);
      }
      if (errno) {
        fprintf(
            stderr,
            "reverse: unable to make sure input and output files differ: %s\n",
            strerror(errno));
        exit(EXIT_FAILURE);
      }
      // -----------------------------------------------------------------------

      if (same) {
        fprintf(stderr, "reverse: input and output file must differ\n");
        fclose(input_fp);
        exit(EXIT_FAILURE);
      }

      output_fp = fopen(outpath, "w");
      if (!output_fp) {
        fprintf(stderr, "reverse: cannot open file '%s'\n", outpath);
        fclose(input_fp);
        exit(EXIT_FAILURE);
      }
    }
    is_redirected = true;
  }

  // (3) Store lines
  SINGLELINE *head = NULL;
  char buffer[BUFSIZ];
  char *line = NULL;
  size_t linelen = 0;

  while (fgets(buffer, BUFSIZ, input_fp) != NULL) {
    // Custom getline() logic: get one line however large it is.
    size_t chunklen = strlen(buffer);
    // Using realloc() the idiomatic way preventing memory leaks.
    char *temp = realloc(line, linelen + chunklen + 1);
    if (!temp) {
      free(line);
      perror("realloc() failed");
      exit(EXIT_FAILURE);
    } else {
      line = temp;
    }
    // Cobble preceding chunk together with the whole line
    memcpy(line + linelen, buffer, chunklen + 1);
    linelen += chunklen;

    // If we have a line or reached the end of the file
    if (buffer[chunklen - 1] == '\n' || feof(input_fp)) {
      // Build list allocating memory for each node.
      SINGLELINE *node = malloc(sizeof(*node));
      if (!node) {
        perror("malloc() failed");
        exit(EXIT_FAILURE);
      }
      node->line = line;
      node->next = head; // Knit list in reverse order,
      head = node;       // prepending each new node at the head.

      // TRICK: my custom getline(): Ensure fresh malloc for the next line
      line = NULL;
      linelen = 0;
    }
  }
  // Make sure the stream file was properly depleted
  if (ferror(input_fp)) {
    fprintf(stderr, "reverse: Unable to read the whole file: %s",
            strerror(errno));
    exit(EXIT_FAILURE);
  }

  // (4) Dump lines
  for (SINGLELINE *curr = head; curr; curr = curr->next) {
    fprintf(output_fp, "%s", curr->line);
  }

  // (5) Clean up: close stream files and free memory
  if (is_redirected) {
    fclose(input_fp);
    if (argc > 2)
      fclose(output_fp);
  }
  for (SINGLELINE *curr = head; curr; curr = head) {
    head = curr->next;
    free(curr->line);
    free(curr);
  }

  return EXIT_SUCCESS;
}
