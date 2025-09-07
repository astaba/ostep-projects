/* ostep-projects/initial-utilities/wzip/wzip.c */
// Created on: Sun Sep  7 04:57:26 +01 2025
// First draft version:
// - Passed all tests
// - Extremely slow

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  if (argc == 1) {
    /* printf("wzip: file1 [file2 ...]\n"); */
    fprintf(stderr, "wzip: file1 [file2 ...]\n");
    exit(EXIT_FAILURE);
  }

  uint32_t counter = 1;
  uint8_t buf[2];
  for (size_t i = 1; i < argc; i++) {
    FILE *fp = fopen(argv[i], "rb");
    if (fp == NULL) {
      fprintf(stderr, "wzip: failed open '%s': %s\n", argv[i], strerror(errno));
      exit(EXIT_FAILURE);
    }

    size_t nread = 0;

    while (1) {
      nread = fread(buf, sizeof(uint8_t), 1, fp);
      if (i == 1 && ftell(fp) == 1) {
        buf[1] = buf[0];
        continue;
      }
      if (nread && buf[0] == buf[1]) {
        counter++;
        buf[1] = buf[0];
        continue;
      } else {

        if (!nread && buf[1] == buf[0] && i < argc - 1) {
          buf[1] = buf[0];
          break;
        }
        fwrite(&counter, sizeof(uint32_t), 1, stdout);
        fwrite(buf + 1, sizeof(uint8_t), 1, stdout);
        buf[1] = buf[0];
        counter = 1;
      }
      if (!nread)
        break;
    }
    fclose(fp);
  }

  return EXIT_SUCCESS;
}
