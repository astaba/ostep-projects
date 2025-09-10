/* ostep-projects/initial-kv/kv.c */
// Created on: Wed Sep 10 03:09:52 +01 2025

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct key {
  int key;
  struct key *next;
} keys_t;
typedef struct keyValue {
  int key;
  char *value;
  struct keyValue *next;
} keyvalues_t;

typedef struct {
  keyvalues_t *put;
  keys_t *get;
  keys_t *delete;
  bool clear;
  bool all;
} Options_t;

void storekeys(keys_t **head, int key) {
  keys_t *node = malloc(sizeof(*node));
  if (!node) {
    perror("malloc failed to allocated keys_t node");
    exit(EXIT_FAILURE);
  }
  node->key = key;
  node->next = NULL;

  if (*head == NULL || (*head)->key > node->key) {
    node->next = *head;
    *head = node;
    return;
  }
  keys_t *curr = (*head);
  while (curr->next && curr->next->key <= node->key) {
    if (curr->next->key == node->key) {
      fprintf(stderr, "kv: Too many keys for g option: %d\n", node->key);
      exit(EXIT_FAILURE);
    }

    curr = curr->next;
  }
  node->next = curr->next;
  curr->next = node;
}

void storekeyvalues(keyvalues_t **head, int key, char *value) {
  keyvalues_t *node = malloc(sizeof(*node));
  if (!node) {
    perror("malloc failed to allocated keyvalues_t node");
    exit(EXIT_FAILURE);
  }
  node->key = key;
  node->value = value;
  node->next = NULL;

  if (*head == NULL || (*head)->key > node->key) {
    node->next = *head;
    *head = node;
    return;
  }
  keyvalues_t *curr = (*head);
  while (curr->next && curr->next->key <= node->key) {
    if (curr->next->key == node->key) {
      curr->next->value = node->value;
      return;
    }

    curr = curr->next;
  }
  node->next = curr->next;
  curr->next = node;
}

int main(int argc, char *argv[]) {
  /* Code */
  // TODO: Validate arguments number

  Options_t args = {NULL, NULL, NULL, false, false};

  for (size_t i = 1; i < argc; i++) {
    char *cmd = strsep(&argv[i], ",");
    char *nptr = strsep(&argv[i], ",");
    char *value = strsep(&argv[i], ",");

    if (!nptr) {
      if (!args.clear && !strcmp(cmd, "c"))
        args.clear = true;
      else if (!args.all && !strcmp(cmd, "a"))
        args.all = true;
    } else {
      char *endptr = NULL;
      errno = 0;
      int key = (int)strtol(nptr, &endptr, 10);
      // Check for various error conditions.
      // 1. Check for overflow/underflow (errno is set).
      if (errno != 0 && key == 0) {
        perror("strtol() failed due to range error");
        exit(EXIT_FAILURE);
      }
      // 2. Check if no digits were found.
      // no conversion: endptr will point to the beginning of nptr
      if (endptr == nptr) {
        fprintf(stderr, "No digits were found in '%s'.\n", nptr);
        exit(EXIT_FAILURE);
      }
      // 3. Check for leftover characters.
      // endptr is not at nptr's end: nptr is not a pure number
      if (*endptr != '\0') {
        fprintf(stderr, "Trailing characters in '%s' after conversion.\n",
                nptr);
        exit(EXIT_FAILURE);
      }

      if (!strcmp(cmd, "g")) {
        storekeys(&(args.get), key);
      } else if (!strcmp(cmd, "d")) {
        storekeys(&(args.delete), key);
      } else if (value && !strcmp(cmd, "p")) {
        storekeyvalues(&(args.put), key, value);
      }
    }
  }

  if (args.get) {
    for (keys_t *curr = args.get; curr; curr = curr->next) {
      printf("Get:\t%d\n", curr->key);
    }
    // Clean up
    for (keys_t *curr = args.get; curr; curr = args.get) {
      args.get = curr->next;
      free(curr);
    }
  }

  if (args.delete) {
    for (keys_t *curr = args.delete; curr; curr = curr->next) {
      printf("Delete:\t%d\n", curr->key);
    }
    // Clean up
    for (keys_t *curr = args.delete; curr; curr = args.delete) {
      args.delete = curr->next;
      free(curr);
    }
  }

  if (args.put) {
    for (keyvalues_t *curr = args.put; curr; curr = curr->next) {
      printf("Put:\t%d %s\n", curr->key, curr->value);
    }
    // Clean up
    for (keyvalues_t *curr = args.put; curr; curr = args.put) {
      args.put = curr->next;
      free(curr);
    }
  }

  return EXIT_SUCCESS;
}
