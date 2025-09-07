// ostep-projects/initial-utilities/wgrep/yygrep.c
// Created on: Sat Sep  6 14:01:26 +01 2025
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef BUFSIZ
#define BUFSIZ 4096
#endif

void mygetline(FILE *stream, const char *searchterm, const char *filename);
char *istrstr(const char *haystack, const char *needle);

/**
 * @brief Main entry point for the ygrep utility.
 * This program searches for a given search term in one or more files, or from
 * standard input if no files are specified.
 * Usage: ygrep <searchterm> [filename ...]
 * Arguments:
 *   argc      The number of command-line arguments.
 *   argv      The array of command-line argument strings.
 * Behavior:
 *   - If fewer than 2 arguments are provided, prints usage information and
 * exits with failure.
 *   - If only the search term is provided, reads from standard input and
 * searches for the term.
 *   - If filenames are provided, opens each file in turn, searches for the
 * term, and prints matching lines.
 *   - If a file cannot be opened, prints an error message and exits with
 * failure.
 * Return: EXIT_SUCCESS on successful completion, EXIT_FAILURE on error.
 */
int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <searchterm> [filename .. .. ..]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  const char *searchterm = argv[1];

  if (argc == 2) {
    mygetline(stdin, searchterm, "stdin");
  } else {
    for (size_t i = 2; i < argc; i++) {
      char *filename = argv[i];
      FILE *fp = fopen(filename, "r");
      if (fp == NULL) {
        fprintf(stderr, "ygrep: cannot open file %s: %s\n", filename,
                strerror(errno));
        exit(EXIT_FAILURE);
      }
      mygetline(fp, searchterm, filename);
      fclose(fp);
    }
  }

  return EXIT_SUCCESS;
}

/**
 * @brief Reads lines from a given file stream, searches for a specified term,
 * and prints matching lines with context.
 * This function reads the input stream line by line, dynamically handling lines
 * of arbitrary length. For each line, it searches for all (case-insensitive)
 * occurrences of the specified search term. When a match is found, it prints
 * the filename, line number, column number, and the full line containing the
 * match. If no matches are found in the entire file, a message is printed
 * indicating no matches.
 * @param stream      Pointer to the input FILE stream to read from.
 * @param searchterm  The string to search for within each line of the file.
 * @param filename    The name of the file being processed (used for output
 * context).
 * @note
 * - The function uses a dynamically allocated buffer to handle lines longer
 * than BUFSIZ.
 * - It assumes the existence of a case-insensitive substring search function
 * `istrstr`.
 * - The function prints output in the format: filename:line:column:line_content
 * - If memory allocation fails, the function prints an error and exits.
 */
void mygetline(FILE *stream, const char *searchterm, const char *filename) {
  // A fixed-size buffer to read chunks of data from the stream.
  char buffer[BUFSIZ];
  size_t lineno = 0, colmno = 0;
  // A dynamically-sized buffer to build the complete line.
  char *linebuf = NULL;
  size_t linelen = 0;
  size_t neelen = strlen(searchterm);

  // Read chunks from the stream until end-of-file or an error.
  while (fgets(buffer, BUFSIZ, stream) != NULL) {
    size_t chunklen = strlen(buffer);

    // Safely reallocate memory for the growing line buffer.
    // This pattern prevents a memory leak if realloc fails.
    char *tmp_buf = realloc(linebuf, linelen + chunklen + 1);
    if (!tmp_buf) {
      perror("realloc() failed");
      free(linebuf);
      exit(EXIT_FAILURE);
    }
    linebuf = tmp_buf;

    // Append the new buffer chunk to the end of the accumulated line buffer.
    // linelen is used as the offset to copy data to the correct location.
    memcpy(linebuf + linelen, buffer, chunklen + 1);
    linelen += chunklen;

    // Check for a newline or end-of-file to determine the end of a complete
    // line.
    if (buffer[chunklen - 1] == '\n' || feof(stream)) {
      lineno++;
      char *p = linebuf;
      // Search for all occurrences of the search term on the complete line.
      while ((p = istrstr(p, searchterm)) != NULL) {
        colmno = 1 + (size_t)(p - linebuf);
        // Print the matched line, including file, line, and column numbers.
        // The ternary operator adds a newline if the line wasn't terminated by
        // one.
        printf("%s:%zu:%zu:%s%s", filename, lineno, colmno, linebuf,
               linebuf[linelen - 1] == '\n' ? "" : "\n");
        /* printf("%s:%zu:%zu:%s\n", filename, lineno, colmno, searchterm); */
        // Advance the pointer past the current match to find the next one.
        p += neelen;
      }

      // Free the buffer for the completed line and reset for the next one.
      free(linebuf);
      linebuf = NULL;
      linelen = 0;
    }
  }

  // Free the final buffer, which is a no-op if the last line was processed
  // correctly but crucial for handling errors and edge cases.
  free(linebuf);

  // Print a message if no matches were found.
  if (colmno == 0) {
    printf("No match in '%s'\n", filename);
  }
}

/**
 * istrstr - Case-insensitive substring search.
 * Searches for the first occurrence of the substring 'needle' in the string
 * 'haystack', ignoring the case of both strings. The search is case-insensitive
 * and returns a pointer to the beginning of the located substring, or NULL if
 * the substring is not found.
 * @param haystack The string to be searched.
 * @param needle   The substring to search for.
 * @return         A pointer to the first occurrence of 'needle' in 'haystack'
 * (case-insensitive), or NULL if 'needle' is not found or is an empty string.
 */
char *istrstr(const char *haystack, const char *needle) {
  if (!*needle)
    return NULL;

  for (; *haystack; ++haystack) {
    if (toupper((unsigned char)*haystack) == toupper((unsigned char)*needle)) {
      const char *h, *n;

      for (h = haystack, n = needle; *h && *n; ++h, ++n)
        if (toupper((unsigned char)*h) != toupper((unsigned char)*n))
          break;

      if (!*n)
        return (char *)haystack;
    }
  }
  return NULL;
}
