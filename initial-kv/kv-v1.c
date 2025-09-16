/* ostep-projects/initial-kv/kv-v1.c */
// =============================================================================
// Program: Simple Key-Value Store
// Created on: Tue Sep 16 22:19:08 +01 2025
// Author: <Your Name>
// =============================================================================
// Description: This program implements a lightweight persistent key-value store
// backed by a text file. Data is stored in the form: key,value
// The store supports the following operations:
//   - a: Get all key-value pairs
//   - c: Clear all key-value pairs
//   - d,<key>: Delete entry by key
//   - g,<key>: Get single entry by key
//   - p,<key>,<value>: Insert or update entry
// Persistence is handled via atomic file replacement using `rename()`.
// Build: gcc -Wall -Wextra -pedantic -std=c11 -o kvstore kvstore.c
// Usage: ./kvstore <option>,[key],[value]...
// Example: ./kvstore p,1,hello g,1
// =============================================================================

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// =============================================================================
// Data Structures
// =============================================================================

/**
 * @struct keyValue
 * @brief A node in the linked list storing a key-value pair.
 * Each node contains:
 *   - `key`: integer identifier
 *   - `value`: dynamically allocated string
 *   - `next`: pointer to the next node
 */
typedef struct keyValue {
  int key;
  char *value;
  struct keyValue *next;
} kv_t;

// =============================================================================
// Helper Functions (Linked List Management)
// =============================================================================

/**
 * @brief Manage the key-value store operations.
 *
 * Supported operations:
 *   - 'a': Print all key-value pairs.
 *   - 'c': Clear all key-value pairs.
 *   - 'd': Delete a key-value pair by key.
 *   - 'g': Get and print a single key-value pair by key.
 *   - 'p': Insert or update a key-value pair.
 *
 * @param head   Pointer to the head pointer of the linked list.
 * @param option Single-character string representing the operation.
 * @param key    Key associated with the operation.
 * @param value  Value string for 'p' (put) operations (may be NULL otherwise).
 */
void dbmanager(kv_t **head, const char *option, int key, const char *value) {
  switch (option[0]) {
  case 'a': { // Get all key-value pairs
    if (*head == NULL) {
      printf("Database is empty.\n");
    } else {
      for (kv_t *curr = *head; curr; curr = curr->next) {
        printf("%d,%s\n", curr->key, curr->value);
      }
    }
    break;
  }
  case 'c': { // Clear all key-value pairs
    printf("Clearing database.\n");
    kv_t *curr = *head;
    while (curr) {
      kv_t *temp = curr;
      curr = curr->next;
      free(temp->value);
      free(temp);
    }
    *head = NULL;
    break;
  }
  case 'd': { // Delete a single key-value pair
    kv_t *curr = *head;
    kv_t *prev = NULL;

    while (curr && curr->key != key) {
      prev = curr;
      curr = curr->next;
    }

    if (!curr) {
      fprintf(stderr, "Could not delete entry %d: No such entry.\n", key);
    } else {
      if (prev) {
        prev->next = curr->next;
      } else {
        *head = curr->next;
      }
      printf("Deleting: %d,%s\n", curr->key, curr->value);
      free(curr->value);
      free(curr);
    }
    break;
  }
  case 'g': { // Get a single key-value pair
    kv_t *curr = *head;
    while (curr && curr->key != key) {
      curr = curr->next;
    }
    if (curr) {
      printf("%d,%s\n", curr->key, curr->value);
    } else {
      fprintf(stderr, "%d not found.\n", key);
      /* // TEST: for test 3
      printf("%d not found\n", key); */
    }
    break;
  }
  case 'p': { // Put/update a single key-value pair
    kv_t *curr = *head;
    kv_t *prev = NULL;

    // Allocate and copy the value to the heap to ensure it persists.
    char *heap_value = strdup(value);
    if (!heap_value) {
      perror("strdup failed");
      exit(EXIT_FAILURE);
    }

    while (curr && curr->key <= key) {
      if (curr->key == key) { // Key already exists, update value
        free(curr->value);
        curr->value = heap_value;
        return;
      }
      prev = curr;
      curr = curr->next;
    }

    // Key does not exist, so insert a new node
    kv_t *node = malloc(sizeof(*node));
    if (!node) {
      perror("malloc failed");
      exit(EXIT_FAILURE);
    }
    node->key = key;
    node->value = heap_value;
    node->next = NULL;

    if (!prev) { // Prepend at the head
      node->next = *head;
      *head = node;
    } else { // Insert in the list
      node->next = prev->next;
      prev->next = node;
    }
    break;
  }
  }
}
// =============================================================================

/**
 * @brief Free all memory allocated for the key-value linked list.
 *
 * @param head Pointer to the head pointer of the list.
 */
void free_db(kv_t **head) {
  for (kv_t *curr = *head; curr; curr = *head) {
    *head = curr->next;
    free(curr->value);
    free(curr);
  }
}
// =============================================================================
// File I/O Functions
// =============================================================================

/**
 * @brief Load the key-value store from a file.
 *
 * The file is expected to contain one key-value pair per line,
 * formatted as "key,value".
 *
 * @param head     Pointer to the head pointer of the linked list.
 * @param filename Path to the database file.
 */
void load_db_from_file(kv_t **head, const char *filename) {
  FILE *db_fp = fopen(filename, "r");
  if (!db_fp) {
    // This is a normal condition if the file doesn't exist yet.
    // Therefore better to fail silently
    return;
  }

  char *line = NULL;
  size_t len = 0;
  ssize_t read_bytes;

  while ((read_bytes = getline(&line, &len, db_fp)) != -1) {
    line[strcspn(line, "\n")] = '\0';
    char *linecpy = strdup(line);
    if (!linecpy) {
      perror("strdup failed");
      free(line);
      fclose(db_fp);
      exit(EXIT_FAILURE);
    }
    char *ptr_cpy = linecpy; // Store the original pointer

    char *nptr = strsep(&linecpy, ",");
    char *value = strsep(&linecpy, ",");

    if (nptr && value) {
      char *endptr = NULL;
      errno = 0;
      int key = (int)strtol(nptr, &endptr, 10);
      if (errno == ERANGE || *endptr != '\0') {
        fprintf(stderr, "Error: Database corrupted key '%s': %s", nptr,
                strerror(errno));
        free(line);
        free(ptr_cpy);
        // WARNING: STUDY CASE: How to thoroughly free memory before exit.
        // This one still leaves one block behind.
        free_db(head);
        exit(EXIT_FAILURE);
      }

      dbmanager(head, "p", key, value);
    }
    free(ptr_cpy);
  }
  free(line);
  fclose(db_fp);
}
// -----------------------------------------------------------------------------

/**
 * @brief Save the key-value store to a file atomically.
 *
 * Writes to a temporary file first, then uses `rename()` to replace
 * the old database. This ensures that the database is never corrupted
 * by partial writes.
 *
 * If the database is empty, the target file is unlinked (removed).
 *
 * @param head     Pointer to the head pointer of the linked list.
 * @param filename Path to the database file.
 */
void save_db_to_file(kv_t **head, const char *filename) {
  if (!*head) {
    unlink(filename);
    return;
  }

  char temp_filename[256];
  snprintf(temp_filename, sizeof(temp_filename), "/tmp/kvXXXXXX%s", filename);

  int temp_fd = mkstemps(temp_filename, strlen(filename));
  if (temp_fd == -1) {
    perror("Failed to create temporary file");
    exit(EXIT_FAILURE);
  }

  FILE *temp_fp = fdopen(temp_fd, "w");
  if (!temp_fp) {
    perror("Failed to open temporary file in text-writing");
    close(temp_fd);
    unlink(temp_filename);
    exit(EXIT_FAILURE);
  }

  for (kv_t *curr = *head; curr; curr = curr->next) {
    fprintf(temp_fp, "%d,%s\n", curr->key, curr->value);
  }
  fclose(temp_fp);

  // NOTE: How rename() works?
  // Think of it like moving a photograph into a picture frame. When you place
  // the new photo in the frame, the old one is automatically removed. You don't
  // need a separate step to throw away the old one; it's part of the process of
  // putting the new one in place.
  // This atomic rename pattern is the standard and safest way to update a file
  // on disk. It guarantees that at no point in the process is your data file
  // corrupt or half-written. The file either exists in its original state or
  // the new, fully written state.

  if (rename(temp_filename, filename) != 0) {
    perror("Failed to rename temporary file");
    unlink(temp_filename);
    exit(EXIT_FAILURE);
  }
}
// =============================================================================
// Utility Functions
// =============================================================================

/**
 * @brief Print program usage instructions and exit.
 *
 * @param prog_name Name of the executable (argv[0]).
 */
void usage(const char *prog_name) {
  fprintf(stderr, "Usage: %s <option>,[key],[value]...\n", prog_name);
  fprintf(stderr, "  Options:\n");
  fprintf(stderr, "    a: get all\n");
  fprintf(stderr, "    c: clear all\n");
  fprintf(stderr, "    d,<key>: delete entry\n");
  fprintf(stderr, "    g,<key>: get single entry\n");
  fprintf(stderr, "    p,<key>,<value>: put/update entry\n");
  exit(EXIT_FAILURE);
}

// =============================================================================
// Main Function
// =============================================================================

/**
 * @brief Program entry point.
 *
 * Loads the key-value database from disk, applies operations specified
 * in command-line arguments, saves the modified database back to disk,
 * and frees memory.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
int main(int argc, char *argv[]) {
  // TEST: for test 1 comment out all the if close.
  if (argc < 2) {
    usage(argv[0]);
  }

  kv_t *dbhead = NULL;
  const char *filename = "database.txt";

  // Load the existing database from file.
  load_db_from_file(&dbhead, filename);

  // Process command invocation options.
  for (int i = 1; i < argc; i++) {
    char *arg_copy = strdup(argv[i]);
    if (!arg_copy) {
      perror("strdup failed");
      free_db(&dbhead);
      exit(EXIT_FAILURE);
    }
    char *ptr_cpy = arg_copy; // Store the original pointer

    char *option = strsep(&arg_copy, ",");
    char *nptr = strsep(&arg_copy, ",");
    char *value = strsep(&arg_copy, ",");
    int key = 0;

    if (!option) {
      fprintf(stderr, "Missing option at argument %d.\n", i);
      free(ptr_cpy);
      continue;
    }

    // Validate option
    const char *valid_opts = "acdgp";
    if (!strchr(valid_opts, option[0]) || strlen(option) != 1) {
      fprintf(stderr, "Bad option '%s' at argument %d. Must be one of: %s\n",
              option, i, valid_opts);
      free(ptr_cpy);
      continue;
    }

    // Validate key for options that need one
    if (option[0] != 'a' && option[0] != 'c') {
      if (!nptr) {
        fprintf(stderr, "Missing key at argument %d.\n", i);
        free(ptr_cpy);
        continue;
      }
      char *endptr = NULL;
      errno = 0;
      key = (int)strtol(nptr, &endptr, 10);
      if (errno == ERANGE || *endptr != '\0') {
        fprintf(stderr, "Invalid key '%s' at argument %d.\n", nptr, i);
        free(ptr_cpy);
        continue;
      }
    }

    // Validate value for put option
    if (option[0] == 'p' && !value) {
      fprintf(stderr, "Missing value at argument %d for 'p' option.\n", i);
      free(ptr_cpy);
      continue;
    }

    dbmanager(&dbhead, option, key, value);
    free(ptr_cpy);
  }

  // Save the modified database to the file.
  save_db_to_file(&dbhead, filename);

  // Free all allocated memory.
  free_db(&dbhead);

  return EXIT_SUCCESS;
}
