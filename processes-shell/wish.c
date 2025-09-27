/* ostep-projects/processes-shell/wish.c */
// Crated on: Thu Sep 18 15:14:14 +01 2025
// INFO: Usage:
// Without custom path only the three builtins cmds are available.
// To fully enjoy the wish shell it is recommended to initialize its custom
// path by with the /bin directory:
// wish> path /bin
// Any subequent path cmd call should either clear the custom path or
// overwrite it like:
// wish> path /bin /usr/bin /home/owner/bin

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

typedef enum { interactive, batch } shmode_t;

char **cleanup_pp_dynamarray(char **arr) {
  if (arr) {
    for (char **ptr = arr; *ptr; ptr++) {
      free(*ptr);
    }
    free(arr);
  }
  return NULL;
}

void print_err_msg(void) {
  char *error_message = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message));
}

int main(int argc, char *argv[argc + 1]) {
  // INFO: If the shell is invoked with more than one file:
  // throw exception and exit(1)
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
  char **path = NULL;
  size_t path_nbr = 0;

  if (argv[1]) {
    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
      fprintf(stderr, "Failed to open '%s': %s\n", argv[1], strerror(errno));
      exit(EXIT_FAILURE);
    }
    fclose(fp);
    // TODO: In batch mode any format error of cmd in the batch file
    // should trigger  exception and exit(1)
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

      // TODO: Run the main logic:
      // 1.✓ Parse the line into a dynamarray.
      // 2. Should account for redirection (chevron >) in the strict format:
      //   - wish> <cmd> [ arg ...] > <file>
      //   - More than one chevron is error;
      //   - More than one file after the chevron is error;
      //   - If the file already exists it must be truncated;
      //   - Builtins cmd do not support redirection (for the moment).
      //   - The implementation in the child process should not only redirect
      //     STDOUT_FILENO but also STDERR_FILENO.
      // 3. Should account for parallel cmds with ampersand (&) in the strict
      // format:
      //   - wish> cmd1 & cmd2 & ... cmdn
      //   - A single parallel cmd could include a redirection and then be
      //     properly parsed out and properly passed down to the child process;
      //   - Therefore the format <cmd> could be:
      //      - <cmd> [args ...]
      //      - <cmd> [args ...] > <file>
      //   - Builtins cmd are not concerned by the parallel implementation;
      //   - After separating each cmd and its optional arguments the parent
      //     should launch them each in its own process and properly wait for
      //     each of them before cleanup and next shell prompt;
      //
      line[strcspn(line, "\n")] = '\0';
      char **dynamargs = NULL;
      char *token = NULL;
      char *ptrcpy = line;
      size_t counter = 0;
      while ((token = strsep(&ptrcpy, "\t "))) {
        if (!strlen(token))
          continue;

        // DEBUG:
        // printf("'%s'\n", token);
        char *tmp = realloc(dynamargs, sizeof(char *) * (counter + 2));
        if (!tmp) {
          perror("realloc() failed");
          dynamargs = cleanup_pp_dynamarray(dynamargs);
          exit(EXIT_FAILURE);
        } else {
          dynamargs = (char **)tmp;
        }
        dynamargs[counter] = strdup(token);
        if (!dynamargs[counter]) {
          perror("strdup() failed");
          dynamargs = cleanup_pp_dynamarray(dynamargs);
          exit(EXIT_FAILURE);
        }
        // DEBUG:
        // printf("dynamargs[%zu]: %s\n", counter - 1, dynamargs[counter - 1]);
        counter++;
        dynamargs[counter] = NULL;
      }

      // INFO: The shell has its own builtins cmd implemented through syscalls:
      // path, cd and exit.
      if (!strcmp(dynamargs[0], "exit")) {
        if (counter > 1) {
          print_err_msg();
          // INFO: Run only one cmd per shell loop iteration.
          continue;
        } else {
          exit(EXIT_SUCCESS);
        }
      } else if (!strcmp(dynamargs[0], "cd")) {
        if (counter != 2) {
          print_err_msg();
        } else {
          if (chdir(dynamargs[1]) == -1) {
            print_err_msg();
            /* perror("chdir() failed"); */
          }
        }
        // INFO: Run only one cmd per shell loop iteration.
        continue;

      } else if (!strcmp(dynamargs[0], "path")) {
        // TODO: The user should able to clear the path.
        if (counter < 2) {
          print_err_msg();
        } else {
          if (path) {
            path = cleanup_pp_dynamarray(path);
            path_nbr = 0;
          }
          path = malloc(sizeof(char *) * counter);
          if (!path) {
            perror("malloc() failed");
            continue;
          }
          for (size_t i = 0; i < (counter - 1); i++) {
            path[i] = strdup(dynamargs[i + 1]);
            if (!path[i]) {
              perror("strdup failed");
              continue;
            }
          }
          path_nbr = counter - 1;
          // DEBUG
          for (size_t i = 0; i < (counter - 1); i++) {
            printf("path[%zu]: %s\n", i, path[i]);
          }
        }
        // INFO: Run only one cmd per shell loop iteration.
        continue;
      }

      // INFO: If No path: run only builtins cmd.
      if (path) {
        // TODO: Check dynamargs[0] exists in one of the path directories,
        // using tha access() system call.
        // Only then, update dynamargs[0] to the relevant fully qualified
        // path and use execv() in the child process (no more execvp()).
        int in_path = 0;
        for (size_t i = 0; i < path_nbr; i++) {
          char pathname[256];
          snprintf(pathname, 256, "%s/%s", path[i], dynamargs[0]);
          if (!access(pathname, X_OK)) {
            free(dynamargs[0]);
            dynamargs[0] = strdup(pathname);
            // DEBUG:
            // printf("valid pathname: %s\n", dynamargs[0]);
            in_path = 1;
            break;
          }
        }
        // INFO: If the cmd arg is not in any paths:
        // throw error and proceed to the shell prompt.
        if (!in_path) {
          print_err_msg();
          continue;
        }

        pid_t rc = fork();
        if (rc == -1) {
          perror("fork() failed");
          for (size_t j = 0; j < counter; j++) {
            free(dynamargs[j]);
          }
          free(dynamargs);
          exit(EXIT_FAILURE);
        } else if (rc == 0) { // Child process
          execv(dynamargs[0], dynamargs);
          // We free the memory we allocated only if execv() fails.
          // All elements but the last (char *)NULL where allocated with
          // strdup()
          perror("execv() failed to execute");
          dynamargs = cleanup_pp_dynamarray(dynamargs);
          exit(EXIT_FAILURE);
        } else { // Parent process
          pid_t wpid = waitpid(rc, NULL, 0);
          assert(rc == wpid);
          dynamargs = cleanup_pp_dynamarray(dynamargs);
        }
      }
      // INFO: After all processes are done return control to the user through
      // the shell prompt or if in batch mode proceed to the next line if any.
    }
    free(line);
  } // TODO: Otherwise call the main logic right away

  return EXIT_SUCCESS;
}
