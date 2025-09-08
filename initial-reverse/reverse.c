/* ostep-projects/initial-reverse/reverse.c */
// Created on: Mon Sep  8 16:16:55 +01 2025

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct singleline {
  char *line;
  struct singleline *next;
} SINGLELINE;
size_t storelines(FILE *stream, SINGLELINE **head);
void displayline(FILE *stream, SINGLELINE *head) {
  SINGLELINE *currnode = head;
  /* while (currnode != NULL) {
    printf("%s", currnode->line);
    currnode = currnode->next;
  } */
  while (currnode != NULL) {
    char *ptr = currnode->line;
    size_t len = strlen(ptr);
    fwrite(ptr, 1, len, stream);
    currnode = currnode->next;
  }
}

int main(int argc, char *argv[]) {
  if (argc > 3) {
    fprintf(stderr, "usage: reverse <input> <output>\n");
    exit(EXIT_FAILURE);
  }

  SINGLELINE *head = NULL;

  if (argc == 1) {
    // TODO:
  } else {
    char *infile = argv[1];
    FILE *fpi = fopen(infile, "r");
    if (fpi == NULL) {
      fprintf(stderr, "reverse: cannot open file '%s'\n", infile);
      exit(EXIT_FAILURE);
    }

    if (storelines(fpi, &head) == EXIT_SUCCESS) {
      if (argc == 2) {
        displayline(stdout, head);
      } else {
        // FIX: Make sure input file and output file are not the same.
        // strcmp would be reliable only after bash path expansion.
        char *outfile = argv[2];
        if (!strcmp(infile, outfile)) {
          fprintf(stderr, "Input and output file must differ\n");
          exit(EXIT_FAILURE);
        }

        FILE *fpo = fopen(outfile, "w");
        if (fpo == NULL) {
          fprintf(stderr, "reverse: cannot open file '%s'\n", outfile);
          exit(EXIT_FAILURE);
        }
        displayline(fpo, head);
        fclose(fpo);
      }
    }
    fclose(fpi);
  }

  if (head != NULL) {
    SINGLELINE *currnode = head;
    while (currnode != NULL) {
      SINGLELINE *tmpnode = currnode->next;
      free(currnode->line);
      free(currnode);
      currnode = tmpnode;
    }
  }
  return EXIT_SUCCESS;
}

size_t storelines(FILE *stream, SINGLELINE **head) {
  char buffer[4096];
  char *lineptr = NULL;
  size_t wholelen = 0;

  while (fgets(buffer, 4096, stream) != NULL) {
    size_t chunklen = strlen(buffer);

    char *tmp_ptr = realloc(lineptr, wholelen + chunklen + 1);
    if (tmp_ptr == NULL) {
      perror("realloc() failed");
      free(lineptr);
      exit(EXIT_FAILURE);
    } else {
      lineptr = tmp_ptr;
    }

    memcpy(lineptr + wholelen, buffer, chunklen + 1);
    wholelen += chunklen;

    if (buffer[chunklen - 1] == '\n' || feof(stream)) {
      // build linked list
      SINGLELINE *newnode = calloc(1, sizeof(SINGLELINE));
      if (newnode == NULL) {
        perror("calloc() failed");
        return EXIT_FAILURE;
      }
      newnode->line = lineptr;
      newnode->next = *head;
      *head = newnode;

      lineptr = NULL;
      wholelen = 0;
    }
  }

  if (ferror(stream)) {
    fprintf(stderr, "reverse: Unable to get line: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
