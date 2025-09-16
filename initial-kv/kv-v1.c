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
// Data Structures
// =============================================================================
typedef struct keyValue {
  int key;
  char *value;
  struct keyValue *next;
} keyvalues_t;

// =============================================================================
// Helper Functions (Linked List Management)
// =============================================================================
/**
 * @brief Manages the key-value store operations.
 * * @param head A pointer to the head of the linked list.
 * @param option A string representing the desired operation ('a', 'c', 'd',
 * 'g', 'p').
 * @param key The integer key for the operation.
 * @param value The value string for 'p' (put) operations.
 */
void dbmanager(keyvalues_t **head, const char *option, int key,
               const char *value) {
  switch (option[0]) {
  case 'a': { // Get all key-value pairs
    if (*head == NULL) {
      printf("Database is empty.\n");
    } else {
      for (keyvalues_t *curr = *head; curr; curr = curr->next) {
        printf("%d,%s\n", curr->key, curr->value);
      }
    }
    break;
  }
  case 'c': { // Clear all key-value pairs
    printf("Clearing database.\n");
    keyvalues_t *curr = *head;
    while (curr) {
      keyvalues_t *temp = curr;
      curr = curr->next;
      free(temp->value);
      free(temp);
    }
    *head = NULL;
    break;
  }
  case 'd': { // Delete a single key-value pair
    keyvalues_t *curr = *head;
    keyvalues_t *prev = NULL;

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
    keyvalues_t *curr = *head;
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
    keyvalues_t *curr = *head;
    keyvalues_t *prev = NULL;

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
    keyvalues_t *node = malloc(sizeof(*node));
    if (!node) {
      perror("malloc failed");
      exit(EXIT_FAILURE);
    }
    node->key = key;
    node->value = heap_value;
    node->next = NULL;

    if (prev) {
      node->next = prev->next;
      prev->next = node;
    } else { // Insert at the head
      node->next = *head;
      *head = node;
    }
    break;
  }
  }
}

/**
 * @brief Frees all memory allocated for the linked list.
 * * @param head A pointer to the head of the list.
 */
void free_db(keyvalues_t **head) {
  keyvalues_t *curr = *head;
  while (curr) {
    keyvalues_t *temp = curr;
    curr = curr->next;
    free(temp->value);
    free(temp);
  }
  *head = NULL;
}

// =============================================================================
// File I/O Functions
// =============================================================================
/**
 * @brief Loads the key-value store from a file.
 * * @param head A pointer to the head of the linked list.
 * @param filename The name of the database file.
 */
void load_db_from_file(keyvalues_t **head, const char *filename) {
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

    char *nptr = strsep(&linecpy, ",");
    char *value = strsep(&linecpy, ",");

    if (nptr && value) {
      int key = atoi(nptr);
      dbmanager(head, "p", key, value);
    }
    free(linecpy);
  }
  free(line);
  fclose(db_fp);
}

/**
 * @brief Saves the key-value store to a file using a temporary file.
 * * @param head A pointer to the head of the linked list.
 * @param filename The name of the database file.
 */
void save_db_to_file(keyvalues_t **head, const char *filename) {
  if (!*head) {
    unlink(filename);
    return;
  }

  char temp_filename[256];
  snprintf(temp_filename, sizeof(temp_filename), "/tmp/dbXXXXXX%s", filename);

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

  for (keyvalues_t *curr = *head; curr; curr = curr->next) {
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
// Main and Argument Parsing
// FIX: Memory leaks: run valgrind
// =============================================================================
int main(int argc, char *argv[]) {
  // TEST: for test 1 comment out all the if close.
  if (argc < 2) {
    usage(argv[0]);
  }

  keyvalues_t *dbhead = NULL;
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

    char *option = strsep(&arg_copy, ",");
    char *nptr = strsep(&arg_copy, ",");
    char *value = strsep(&arg_copy, ",");
    int key = 0;

    if (!option) {
      fprintf(stderr, "Missing option at argument %d.\n", i);
      free(arg_copy);
      continue;
    }

    // Validate option
    const char *valid_opts = "acdgp";
    if (!strchr(valid_opts, option[0]) || strlen(option) != 1) {
      fprintf(stderr, "Bad option '%s' at argument %d. Must be one of: %s\n",
              option, i, valid_opts);
      free(arg_copy);
      continue;
    }

    // Validate key for options that need one
    if (option[0] != 'a' && option[0] != 'c') {
      if (!nptr) {
        fprintf(stderr, "Missing key at argument %d.\n", i);
        free(arg_copy);
        continue;
      }
      char *endptr = NULL;
      errno = 0;
      key = (int)strtol(nptr, &endptr, 10);
      if (errno == ERANGE || *endptr != '\0') {
        fprintf(stderr, "Invalid key '%s' at argument %d.\n", nptr, i);
        free(arg_copy);
        continue;
      }
    }

    // Validate value for put option
    if (option[0] == 'p' && !value) {
      fprintf(stderr, "Missing value at argument %d for 'p' option.\n", i);
      free(arg_copy);
      continue;
    }

    dbmanager(&dbhead, option, key, value);
    free(arg_copy);
  }

  // Save the modified database to the file.
  save_db_to_file(&dbhead, filename);

  // Free all allocated memory.
  free_db(&dbhead);

  return EXIT_SUCCESS;
}
