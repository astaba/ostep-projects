/* ostep-projects/initial-utilities/wzip/wzip.c */
// Created on: Sun Sep  7 16:58:06 +01 2025

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * wzip.c - Simple file compressor using run-length encoding (RLE).
 *
 * This program reads one or more files specified as command-line arguments,
 * compresses their contents using run-length encoding, and writes the compressed
 * output to stdout. Each run of consecutive identical bytes is replaced by a
 * 4-byte count (uint32_t) followed by the byte value (uint8_t).
 *
 * Usage:
 *   wzip file1 [file2 ...]
 *
 * If no files are provided, the program prints a usage message and exits.
 * If any file cannot be opened, an error message is printed and the program exits.
 *
 * The program processes files in order, treating them as a single continuous stream.
 * For each run of identical bytes, it outputs the count and the byte value.
 */
int main(int argc, char *argv[]) {
  if (argc == 1) {
    printf("wzip: file1 [file2 ...]\n");
    exit(EXIT_FAILURE);
  }

  uint32_t count = 0;
  int prev_byte = EOF;

  for (size_t i = 1; i < argc; i++) {
    FILE *fp = fopen(argv[i], "rb");
    if (fp == NULL) {
      fprintf(stderr, "wzip: failed open '%s': %s\n", argv[i], strerror(errno));
      exit(EXIT_FAILURE);
    }

    int curr_byte;
    while ((curr_byte = fgetc(fp)) != EOF) {
      if (curr_byte == prev_byte) {
        count++;
      } else {
        if (prev_byte != EOF) {
          fwrite(&count, sizeof(uint32_t), 1, stdout);
          fwrite(&prev_byte, sizeof(uint8_t), 1, stdout);
        }
        prev_byte = curr_byte;
        count = 1;
      }
    }
    fclose(fp);
  }

  if (prev_byte != EOF) {
    fwrite(&count, sizeof(uint32_t), 1, stdout);
    fwrite(&prev_byte, sizeof(uint8_t), 1, stdout);
  }

  return EXIT_SUCCESS;
}
