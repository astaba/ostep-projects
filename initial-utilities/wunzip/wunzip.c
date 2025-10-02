/* ostep-projects/initial-utilities/wunzip/wunzip.c */
// Created on: Sun Sep  7 17:50:57 +01 2025

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * wunzip.c - A simple decompression utility for a custom run-length encoded
 * format.
 *
 * Usage:
 *   wunzip file1 [file2 ...]
 *
 * For each input file, this program reads a sequence of (count, ascii) pairs,
 * where 'count' is a 4-byte unsigned integer (unsigned int) and 'ascii' is a
 * 1-byte unsigned integer (unsigned char). For each pair, it writes 'count'
 * copies of the character 'ascii' to standard output.
 *
 * If no files are provided, or if an error occurs while opening or reading a
 * file, an error message is printed and the program exits with a failure
 * status.
 *
 * The program processes each file in order, decompressing their contents to
 * stdout. It uses a buffer to efficiently write repeated characters in chunks.
 *
 * Error handling:
 *   - Prints usage if no files are specified.
 *   - Prints an error and exits if a file cannot be opened.
 *   - Prints an error and exits if the input file is corrupted or incomplete.
 */
int main(int argc, char *argv[]) {
  if (argc == 1) {
    printf("wunzip: file1 [file2 ...]\n");
    exit(EXIT_FAILURE);
  }

  for (size_t i = 1; i < argc; i++) {
    FILE *fp = fopen(argv[i], "rb");
    if (!fp) {
      fprintf(stderr, "wunzip: open failed %s: %s\n", argv[i], strerror(errno));
      exit(EXIT_FAILURE);
    }

    unsigned int count;
    unsigned char ascii;
    while (fread(&count, sizeof(unsigned int), 1, fp) == 1) {
      if (fread(&ascii, sizeof(unsigned char), 1, fp) != 1) {
        fprintf(stderr, "wunzip: corrupted input from '%s': %s\n", argv[i],
                strerror(errno));
        exit(EXIT_FAILURE);
      }

      // TRICK: Use a buffer to efficiently write repeated characters in chunks.
      char outbuf[4096];
      memset(outbuf, ascii, sizeof(outbuf));
      while (count > 0) {
        size_t chunk = count < sizeof(outbuf) ? count : sizeof(outbuf);
        fwrite(outbuf, 1, chunk, stdout);
        count -= chunk;
      }
    }

    fclose(fp);
  }

  return EXIT_SUCCESS;
}
