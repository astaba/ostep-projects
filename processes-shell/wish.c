/* ostep-projects/processes-shell/wish.c */
// Crated on: Thu Sep 18 15:14:14 +01 2025

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

typedef enum { interactive, batch } shmode_t;

int main(int argc, char *argv[argc + 1]) {
  if (argc > 2) {
    fprintf(
        stderr,
        "ERROR: Too much arguments.\n\n"
        "Usage:\n\t%s [batch_file]\n\n"
        "The unique optional argument is a batch file containing the command\n"
        "to run before exit. Without argument run the shell in interactive\n"
        "mode entering command on the prompt. To exit press CTRL-D.\n\n",
        argv[0]);
    exit(EXIT_FAILURE);
  }
  shmode_t mode = interactive;

  if (argv[1]) {
    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
      fprintf(stderr, "Failed to open '%s': %s\n", argv[1], strerror(errno));
      exit(EXIT_FAILURE);
    }
    fclose(fp);
    mode = batch;
  }

  // If interactive: run the prompt loop before calling the main logic
  if (mode == interactive) {
    char *line = NULL;
    size_t len = 0;
    while (1) {
      printf("wish> ");
      ssize_t nread = getline(&line, &len, stdin);
      if (nread == -1) { // Terminate wish shell
        puts("");
        break;
      }
      char **dynamargs = malloc(sizeof(dynamargs));
      if (!dynamargs) {
        perror("malloc() failed");
        exit(EXIT_FAILURE);
      }
      // TODO: Run the main logic:
      // 1. Parse the line into a dynamarray.
      line[strcspn(line, "\n")] = '\0';
      char *token = NULL;
      char *ptrcpy = line;
      size_t counter = 0;
      while ((token = strsep(&ptrcpy, "\t "))) {
        if (!strlen(token))
          continue;

        /* printf("'%s'\n", token); */
        char *tmp = realloc(dynamargs, sizeof(dynamargs) + sizeof(char *));
        if (!tmp) {
          perror("realloc() failed");
          for (size_t j = 0; j < counter; j++) {
            free(dynamargs[j]);
          }
          free(dynamargs);
          exit(EXIT_FAILURE);
        } else {
          dynamargs = (char **)tmp;
        }
        dynamargs[counter++] = strdup(token);
      }

      dynamargs[counter] = NULL;

      pid_t rc = fork();
      if (rc == -1) {
        perror("fork() failed");
        for (size_t j = 0; j < counter; j++) {
          free(dynamargs[j]);
        }
        free(dynamargs);
        exit(EXIT_FAILURE);
      } else if (rc == 0) { // Child process
        execvp(dynamargs[0], dynamargs);
      } else { // Parent process
        pid_t wpid = waitpid(rc, NULL, 0);
        assert(rc == wpid);
      }
    }
    free(line);
  } // TODO: Otherwise call the main logic right away

  return EXIT_SUCCESS;
}
