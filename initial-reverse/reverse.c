/* ostep-projects/initial-reverse/reverse.c */
// Created on: Mon Sep  8 16:16:55 +01 2025

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef struct singleline {
  char *line;
  struct singleline *next;
} SINGLELINE;

size_t storelines(FILE *stream, SINGLELINE **head);
void dumplines(FILE *stream, SINGLELINE *head);
bool is_same_file(const char *path1, const char *path2);

int main(int argc, char *argv[]) {
  if (argc > 3) {
    fprintf(stderr, "usage: reverse <input> <output>\n");
    exit(EXIT_FAILURE);
  }

  SINGLELINE *head = NULL;

  if (argc == 1) {
    if (storelines(stdin, &head) == EXIT_SUCCESS) {
      dumplines(stdout, head);
    }
  } else {
    char *infile = argv[1];
    FILE *fpi = fopen(infile, "r");
    if (fpi == NULL) {
      fprintf(stderr, "reverse: cannot open file '%s'\n", infile);
      exit(EXIT_FAILURE);
    }

    if (storelines(fpi, &head) == EXIT_SUCCESS) {

      if (argc == 2) {
        dumplines(stdout, head);
      } else {
        char *outfile = argv[2];
        if (is_same_file(infile, outfile)) {
          fprintf(stderr, "reverse: input and output file must differ\n");
          exit(EXIT_FAILURE);
        }

        FILE *fpo = fopen(outfile, "w");
        if (fpo == NULL) {
          fprintf(stderr, "reverse: cannot open file '%s'\n", outfile);
          exit(EXIT_FAILURE);
        }
        dumplines(fpo, head);
        fclose(fpo);
      }
    }
    fclose(fpi);
  }

  if (head != NULL) {
    for (SINGLELINE *curr = head; curr; curr = head) {
      head = curr->next;
      free(curr->line);
      free(curr);
    }
  }
  return EXIT_SUCCESS;
}

// NOTE: The return status are good but not used meaningfully.
// Could return the number of line read and treat 0 as error.
size_t storelines(FILE *stream, SINGLELINE **head) {
  char buffer[4096];
  char *lineptr = NULL;
  size_t linelen = 0;

  while (fgets(buffer, 4096, stream)) {
    size_t chunklen = strlen(buffer);

    char *tmp = realloc(lineptr, linelen + chunklen + 1);
    if (!tmp) {
      free(lineptr);
      perror("realloc() failed");
      exit(EXIT_FAILURE);
    } else {
      lineptr = tmp;
    }

    memcpy(lineptr + linelen, buffer, chunklen + 1);
    linelen += chunklen;

    if (buffer[chunklen - 1] == '\n' || feof(stream)) {
      // build linked list
      SINGLELINE *node = calloc(1, sizeof(*node));
      if (!node) {
        perror("calloc() failed");
        return EXIT_FAILURE;
      }
      node->line = lineptr;
      node->next = *head; // prepend, making list reverse by construction.
      *head = node;
      // Housekeeping for next line memory allocation.
      lineptr = NULL;
      linelen = 0;
    }
  }

  if (ferror(stream)) {
    fprintf(stderr, "reverse: Unable to get line: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

void dumplines(FILE *stream, SINGLELINE *head) {
  for (SINGLELINE *curr = head; curr; curr = curr->next) {
    fprintf(stream, "%s", curr->line);
  }
}

bool is_same_file(const char *path1, const char *path2) {
  struct stat stat1, stat2;

  if (stat(path1, &stat1) || stat(path2, &stat2)) {
    // WARNING: By returning false this project requirements impose to assume
    // difference in case of stat() failure (may be due to nonexistent file).
    // Comply to pass the test. But that is a dangerous flaw that could entail
    // massive data loss (both files could still be the same but stat() failed
    // simply because of lack of read rights) For safety always assume identity
    // in case of check-fail and return true. Fail-safe is better.
    return false;
  }

  return (stat1.st_dev == stat2.st_dev) && (stat1.st_ino == stat2.st_ino);
}
