/* ostep-projects/initial-utilities/wzip/wzip.c */
// Created on: Sun Sep  7 16:58:37 +01 2025

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  // Check for correct number of command-line arguments.
  if (argc == 1) {
    /* printf("wzip: file1 [file2 ...]\n"); */
    fprintf(stderr, "wzip: file1 [file2 ...]\n");
    exit(EXIT_FAILURE);
  }

  uint32_t count = 0;
  int prev_char = -1;
  int curr_char;

  // Iterate over each file provided as an argument.
  for (int i = 1; i < argc; i++) {
    FILE *fp = fopen(argv[i], "rb");
    if (fp == NULL) {
      fprintf(stderr, "wzip: failed open '%s': %s\n", argv[i], strerror(errno));
      exit(EXIT_FAILURE);
    }

    // Read the file character by character.
    while ((curr_char = fgetc(fp)) != EOF) {
      // This is the very first character read from any file.
      if (prev_char == -1) {
        prev_char = curr_char;
        count = 1;
        continue;
      }

      // If the current character is the same as the previous one, increment the
      // count.
      if (curr_char == prev_char) {
        count++;
      }
      // If the characters are different, write the compressed run and reset.
      else {
        fwrite(&count, sizeof(uint32_t), 1, stdout);
        fwrite(&prev_char, sizeof(uint8_t), 1, stdout);
        prev_char = curr_char;
        count = 1;
      }
    }
    fclose(fp);
  }

  // After all files are processed, write the last compressed run to stdout.
  if (count > 0) {
    fwrite(&count, sizeof(uint32_t), 1, stdout);
    fwrite(&prev_char, sizeof(uint8_t), 1, stdout);
  }

  return EXIT_SUCCESS;
}
