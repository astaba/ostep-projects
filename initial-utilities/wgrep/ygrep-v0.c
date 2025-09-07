// ostep-projects/initial-utilities/wgrep/ygrep.c
// Created on: Sat Sep  6 04:10:29 +01 2025

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Searches for occurrences of a search term in a file stream and prints matching lines.
 *
 * This function reads each line from the given file stream and searches for all occurrences
 * of the specified search term within each line. For every match found, it prints the filename,
 * line number, column number (1-based), and the entire line containing the match. If the line
 * does not end with a newline character, a newline is appended to maintain output formatting.
 *
 * @param stream     Pointer to the file stream to search.
 * @param searchterm The string to search for within the file.
 * @param filename   The name of the file being searched (used for output).
 */
void searchfile(FILE *stream, const char *searchterm, char *filename) {
  char *line = NULL;
  size_t len = 0;
  size_t lineno = 0;
  size_t colmno;
  size_t searchlen = strlen(searchterm);

  while (getline(&line, &len, stream) != -1) {
    lineno++;
    char *p = line;

    while ((p = strstr(p, searchterm)) != NULL) {
      colmno = p - line + 1; // Get 1-based column of the match
      printf("%s:%zu:%zu:%s", filename, lineno, colmno, line);
      // If the line doesn’t end in \n (like last line of file), output
      // formatting might be odd (no newline). A safe bet:
      if (line[strlen(line) - 1] != '\n')
        putchar('\n');
      // Advance p beyond the match for next strstr() call
      p += searchlen;
    }
  }

  free(line);
}

/**
 * @brief Main entry point for the ygrep utility.
 *
 * This function searches for a specified search term in one or more files,
 * or from standard input if no files are provided. It prints lines containing
 * the search term to standard output.
 *
 * Usage:
 *   ygrep <searchterm> [file ...]
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line argument strings.
 *             argv[1] is the search term.
 *             argv[2..argc-1] are optional filenames to search.
 *
 * @return EXIT_SUCCESS (0) on success, or exits with an error code on failure.
 */
int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "ygrep: %s <searchterm> [file ...]\n", argv[0]);
    exit(2);
  }

  const char *searchterm = argv[1];

  if (argc == 2) {
    searchfile(stdin, searchterm, "stdin");
  } else {
    for (size_t i = 2; i < argc; i++) {
      char *filename = argv[i];
      FILE *fp = fopen(filename, "r");
      if (fp == NULL) {
        fprintf(stderr, "ygrep: cannot open file %s: %s\n", filename,
                strerror(errno));
        exit(EXIT_FAILURE);
      }
      searchfile(fp, searchterm, filename);
      fclose(fp);
    }
  }

  return EXIT_SUCCESS;
}
