/* ostep-projects/initial-kv/kv.c */
// Created on: Wed Sep 10 03:09:52 +01 2025

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifndef BUFSIZ
#define BUFSIZ 4096
#endif /* ifndef BUFSIZ */
// =============================================================================
/* typedef struct key {
  int key;
  struct key *next;
} keys_t; */
typedef struct keyValue {
  int key;
  char *value;
  struct keyValue *next;
} keyvalues_t;

/* typedef struct {
  keyvalues_t *put;
  keys_t *get;
  keys_t *delete;
  bool clear;
  bool all;
} Options_t; */
// =============================================================================
// =============================================================================
void dbmanager(keyvalues_t **head, char *option, int key, char *value) {
  // Either of chars in: "acdgp"
  switch (option[0]) {
  case 'a': { // Get all key value
    if (*head == NULL) {
      // FIX: This notification conflict with the first fopen() call error
      // notification
      /* printf("Database empty.\n"); */
    } else {
      for (keyvalues_t *curr = *head; curr; curr = curr->next) {
        printf("%d,%s\n", curr->key, curr->value);
      }
    }
    break;
  }
  case 'c': { // Clear all key value
    printf("Clearing database.\n");
    for (keyvalues_t *curr = *head; curr; curr = *head) {
      *head = curr->next;
      free(curr->value);
      free(curr);
    }
    break;
  }
  case 'd': { // Delete single key
    if (*head == NULL) {
      printf("Could not delete entry %d: Database already empty.\n", key);
      return;
    }

    keyvalues_t *curr = *head;
    if (curr->key == key) {
      *head = curr->next;
      printf("Deleting: %d,%s\n", curr->key, curr->value);
      free(curr->value);
      free(curr);
      return;
    }

    keyvalues_t *prev = NULL;
    while (curr && curr->key != key) {
      prev = curr;
      curr = curr->next;
    }

    if (!curr) {
      printf("Could not delete entry %d: No such entry.\n", key);
    } else {
      prev->next = curr->next;
      printf("Deleting: %d,%s\n", curr->key, curr->value);
      free(curr->value);
      free(curr);
    }
    break;
  }
  case 'g': { // Get single key
    keyvalues_t *curr = NULL;
    for (curr = *head; curr && curr->key != key; curr = curr->next) {
    }
    if (curr) {
      printf("%d,%s\n", curr->key, curr->value);
    } else {
      printf("Could not get entry %d: No such entry.\n", key);
    }
    break;
  }
  case 'p': { // Put single key value
    // WARNING: The incoming value is char* on the stack
    char *heap_value = malloc(strlen(value) + 1);
    if (!heap_value) {
      perror("malloc failed to allocated value memory");
      exit(EXIT_FAILURE);
    }
    strcpy(heap_value, value);

    keyvalues_t *curr = (*head);
    keyvalues_t *prev = NULL;
    while (curr && curr->key <= key) {
      if (curr->key == key) {
        free(curr->value);
        curr->value = heap_value;
        return;
      }
      prev = curr;
      curr = curr->next;
    }

    keyvalues_t *node = malloc(sizeof(*node));
    if (!node) {
      perror("malloc failed to allocated keyvalues_t node");
      exit(EXIT_FAILURE);
    }
    node->key = key;
    node->value = heap_value;
    node->next = NULL;

    if (*head == NULL || (*head)->key > node->key) {
      node->next = *head;
      *head = node;
      return;
    }
    node->next = prev->next;
    prev->next = node;
    break;
  }
  }
}
// =============================================================================
int main(int argc, char *argv[]) {

  // TODO: Validate arguments number

  keyvalues_t *dbhead = NULL;
  // ---------------------------------------------------------------------------
  char *filename = "database.txt";
  FILE *db_fp = fopen(filename, "r");
  if (!db_fp) {
    // FIX: Improve this notification
    perror("Database was empty");
    // exit(EXIT_FAILURE);
  } else {
    // ---------------------------------------------------------------------------
    char buffer[BUFSIZ];
    char *line = NULL;
    size_t linelen = 0;
    while (fgets(buffer, BUFSIZ, db_fp)) {
      size_t chunklen = strlen(buffer);
      char *temp = realloc(line, linelen + chunklen + 1);
      if (!temp) {
        perror("realloc() failed");
        free(line);
        exit(EXIT_FAILURE);
      } else {
        line = temp;
      }

      memcpy(line + linelen, buffer, chunklen + 1);
      linelen += chunklen;

      if (buffer[chunklen - 1] == '\n' || feof(db_fp)) {
        // build database linked list
        line[strcspn(line, "\n")] = '\0';
        char *linecpy_p = line;

        // TODO:
        // 1. Make sure database is not corrupted
        // 2. Try keeping the line pointer to save extra dynamalloc
        // in dbmanager.
        char *nptr = strsep(&linecpy_p, ",");
        char *value = strsep(&linecpy_p, ",");
        int key = atoi(nptr);

        dbmanager(&dbhead, "p", key, value);

        // clean up for next line
        linelen = 0;
        free(line);
        line = NULL;
      }
    } // TODO:Check ferror(db_fp)
    fclose(db_fp);
  }

  // -----------------------------------------------------------------------------
  // Parse command invocation options
  for (size_t i = 1; i < argc; i++) {
    char *option = strsep(&argv[i], ",");
    char *nptr = strsep(&argv[i], ",");
    char *value = strsep(&argv[i], ",");
    int key;

    if (nptr) {
      char *endptr = NULL;
      errno = 0;
      key = (int)strtol(nptr, &endptr, 10);
      // Check for various error conditions.
      // 1. Check for overflow/underflow (errno is set).
      if (errno == ERANGE) {
        perror("strtol() failed due to range error");
        exit(EXIT_FAILURE);
      }
      // 2. Check if no digits were found.
      // no conversion: endptr will point to the beginning of nptr
      if (endptr == nptr) {
        fprintf(stderr, "No digits were found in '%s'.\n", nptr);
        exit(EXIT_FAILURE);
      }
      // 3. Check for leftover characters: Not necessarily an error:
      // endptr is not at nptr's end: nptr is not a pure number
      if (*endptr != '\0') {
        fprintf(stderr, "Trailing characters in '%s' after conversion.\n",
                nptr);
        exit(EXIT_FAILURE);
      }
    }

    if (!strlen(option)) {
      fprintf(stderr, "Error: no initial option at argument %lu\n", (i - 1));
      exit(EXIT_FAILURE);
    }
    char *valid_opts = "acdgp";
    if (strstr(valid_opts, option)) {
      dbmanager(&dbhead, option, key, value);
    }
  }

  // FIX: Bad idea to truncate whole database before updating it
  db_fp = fopen(filename, "w");
  if (!db_fp) {
    perror("fopen() failed");
    exit(EXIT_FAILURE);
  }
  // Update database disk file
  for (keyvalues_t *curr = dbhead; curr; curr = curr->next) {
    fprintf(db_fp, "%d,%s\n", curr->key, curr->value);
  }

  // After an attemp to update the disk file if the db head is null
  // delete the database disk file.
  if (!dbhead) {
    // printf("unlink()\n");
    unlink(filename);
  }

  // Free memory dynamallocated to database linked list
  for (keyvalues_t *curr = dbhead; curr; curr = dbhead) {
    dbhead = curr->next;
    free(curr->value);
    free(curr);
  }

  // Close database stream file
  fclose(db_fp);
  return EXIT_SUCCESS;
}
