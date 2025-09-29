/* ostep-projects/processes-shell/wish.c */
// Crated on: Thu Sep 18 15:14:14 +01 2025
// INFO: Usage:
// Without custom path only the three builtins cmds are available:
// path, cd, exit.
// To fully enjoy the wish shell it is recommended to initialize its custom
// path with the /bin directory:
// wish> path /bin
// Any subsequent path cmd call should either clear the custom path or
// overwrite it like:
// wish> path /bin /usr/bin /home/owner/bin

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

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
  FILE *input_fp = stdin;
  bool batch_mode = false;
  char **dynampaths = NULL;
  size_t path_nbr = 0;

  if (argv[1]) {
    input_fp = fopen(argv[1], "r");
    if (!input_fp) {
      fprintf(stderr, "wish: failed to open '%s': %s\n", argv[1],
              strerror(errno));
      exit(EXIT_FAILURE);
    }
    // TODO: In batch mode any format error of cmd in the batch file
    // should trigger  exception and exit(1)
    batch_mode = true;
  }

  char *line = NULL;
  size_t len = 0;
  while (1) {
    if (!batch_mode) {
      printf("wish> "); // Display prompt
    } else {
      puts(""); // Space line of commands
    }
    ssize_t nread = getline(&line, &len, input_fp);
    if (nread == -1) { // Terminate wish shell
      break;
    }

    line[strcspn(line, "\n")] = '\0'; // Make the input a C standard string

    if (batch_mode) // In batch mode feedback the line of commands
      printf("%s\n", line);

    // WARNING: Make sure to perform memory cleanup required by the scope
    // before jumping to next command(s).

    // Right after reading one input line search parallel commands (&).
    char **dynamcmds = NULL; // The commands table
    // The number of cmds in the cmds table ignoring the terminating NULL
    size_t cmds_nbr = 0;
    char *lineptr_cpy = line;
    char *cmd_token = NULL;
    while ((cmd_token = strsep(&lineptr_cpy, "&"))) {
      // FIX: Make sure to strip away empty lines and invisible characters in
      // the initial part of tokens.
      int empty_len = strspn(cmd_token, " \t\n");
      if (empty_len) {
        cmd_token += empty_len;
      }
      if (!strlen(cmd_token)) { // input line includes "&&"
        print_err_msg();
        continue;
      }
      char *tmp = realloc(dynamcmds, sizeof(char *) * (cmds_nbr + 2));
      if (!tmp) {
        perror("realloc() failed");
        dynamcmds = cleanup_pp_dynamarray(dynamcmds);
        free(line);
        exit(EXIT_FAILURE);
      } else {
        dynamcmds = (char **)tmp;
      }

      dynamcmds[cmds_nbr] = strdup(cmd_token);
      if (!dynamcmds[cmds_nbr]) {
        perror("strdup() failed");
        dynamcmds = cleanup_pp_dynamarray(dynamcmds);
        free(line);
        exit(EXIT_FAILURE);
      }
      cmds_nbr++;
      dynamcmds[cmds_nbr] = NULL;
    }

    if (!dynamcmds)
      continue;
    // WARNING: Confirm cmds table exists before entering next loop

    for (size_t i = 0; dynamcmds[i]; i++) {
      // Count the number of '>' characters in the input cmd and turn on the
      // redirected flag if any.
      bool redirected = false;
      char *char_ptr = dynamcmds[i];
      size_t chevron_count = 0;
      while ((char_ptr = strchr(char_ptr, 0x3E))) {
        char_ptr++;
        chevron_count++;
      }
      // In case of formatting error involving multiple '>' characters in the
      // input cmd ignore the cmd and jump to next cmd.
      if (chevron_count > 1) {
        print_err_msg();
        continue; // Skip this cmd and go to next cmd in the cmds table
      } else if (chevron_count) {
        redirected = true;
      }

      // Parse out actual cmd and optional arguments within an input cmd.
      char **dynamargs = NULL; // Table of cmd elements: cmd [args ...] [> file]
      // Number of elements in the arguments table ignoring the terminating NULL
      size_t args_nbr = 0;
      char *arg_token = NULL;
      char *cmdptr_cpy = dynamcmds[i];
      // Numbers of elements (args) from redirection '>' (including) onwards.
      size_t chevron_onwards = 0;
      // Hypothetic unique argument following the redirection '>'
      char *redirect_path = NULL;

      while ((arg_token = strsep(&cmdptr_cpy, "\t "))) {
        if (!strlen(arg_token))
          continue;

        if (!chevron_onwards) {
          if (!strcmp(arg_token, ">")) {
            // Throw away the chevron and jump to the next argument
            chevron_onwards = 1;
            continue;
          }

          char *tmp = realloc(dynamargs, sizeof(char *) * (args_nbr + 2));
          if (!tmp) {
            perror("realloc() failed");
            dynamargs = cleanup_pp_dynamarray(dynamargs);
            dynamcmds = cleanup_pp_dynamarray(dynamcmds);
            free(line);
            exit(EXIT_FAILURE);
          } else {
            dynamargs = (char **)tmp;
          }
          dynamargs[args_nbr] = strdup(arg_token);
          if (!dynamargs[args_nbr]) {
            perror("strdup() failed");
            dynamargs = cleanup_pp_dynamarray(dynamargs);
            dynamcmds = cleanup_pp_dynamarray(dynamcmds);
            free(line);
            exit(EXIT_FAILURE);
          }
          args_nbr++;
          dynamargs[args_nbr] = NULL;
        } else {
          if (chevron_onwards == 1) {
            redirect_path = strdup(arg_token);
            if (!redirect_path) {
              perror("strdup() failed");
              dynamargs = cleanup_pp_dynamarray(dynamargs);
              dynamcmds = cleanup_pp_dynamarray(dynamcmds);
              free(line);
              exit(EXIT_FAILURE);
            }
          }
          chevron_onwards++;
        }
      }

      // In case of formatting error involving more than one argument after the
      // redirection input cmd cleanup the cmd and jump to next cmd.
      if (redirected) {
        if (chevron_onwards == 1 || chevron_onwards > 2 ||
            strstr("pathcdexit", dynamargs[0])) {
          print_err_msg();
          dynamargs = cleanup_pp_dynamarray(dynamargs);
          free(redirect_path);
          continue; // Skip this cmd and jump to next cmd in cmds table
        }
      }

      if (!redirected) {
        // INFO: The shell has its own builtins cmd implemented through
        // syscalls: path, cd and exit.
        if (!strcmp(dynamargs[0], "exit")) {
          if (args_nbr > 1) {
            print_err_msg();
            dynamargs = cleanup_pp_dynamarray(dynamargs);
            continue; // Jump to next parallel cmd.
          } else {
            // WARNING: Cleanup dynampaths ???
            dynamargs = cleanup_pp_dynamarray(dynamargs);
            dynamcmds = cleanup_pp_dynamarray(dynamcmds);
            free(line);
            dynampaths = cleanup_pp_dynamarray(dynampaths);
            if (batch_mode)
              fclose(input_fp);
            exit(EXIT_SUCCESS);
          }
        } else if (!strcmp(dynamargs[0], "cd")) {
          if (args_nbr != 2) {
            print_err_msg();
          } else {
            if (chdir(dynamargs[1]) == -1) {
              perror("wish cd");
            }
          }
          dynamargs = cleanup_pp_dynamarray(dynamargs);
          continue; // Jump to next parallel cmd.

        } else if (!strcmp(dynamargs[0], "path")) {
          // TODO: The user should able to clear the path.
          if (args_nbr < 2) {
            print_err_msg();
          } else {
            if (dynampaths) { // Purge path before updating it
              dynampaths = cleanup_pp_dynamarray(dynampaths);
              path_nbr = 0;
            }
            dynampaths = malloc(sizeof(char *) * args_nbr);
            if (!dynampaths) {
              perror("malloc() failed");
              dynamargs = cleanup_pp_dynamarray(dynamargs);
              dynamcmds = cleanup_pp_dynamarray(dynamcmds);
              free(line);
              // WARNING: In batch mode cleanup input_fp before exit
              exit(EXIT_FAILURE);
            }
            for (size_t i = 0; i < (args_nbr - 1); i++) {
              dynampaths[i] = strdup(dynamargs[i + 1]);
              if (!dynampaths[i]) {
                perror("strdup failed");
                dynamargs = cleanup_pp_dynamarray(dynamargs);
                dynamcmds = cleanup_pp_dynamarray(dynamcmds);
                dynampaths = cleanup_pp_dynamarray(dynampaths);
                free(line);
                exit(EXIT_FAILURE);
              }
            }
            path_nbr = args_nbr - 1;
            dynampaths[path_nbr] = NULL;
            // DEBUG
            for (size_t i = 0; dynampaths[i]; i++) {
              printf("path[%zu]: %s\n", i, dynampaths[i]);
            }
          }
          dynamargs = cleanup_pp_dynamarray(dynamargs);
          continue; // Jump to next parallel cmd.
        } // if cmd0 == path
      } // if not redirected

      // If no path: run only builtins cmd.
      if (dynampaths) {
        // Check dynamargs[0] exists in one of the path directories, using the
        // access() system call. Only then, update dynamargs[0] to the relevant
        // fully qualified path and use execv() in the child process (no more
        // execvp()).
        bool in_path = false;
        for (size_t i = 0; i < path_nbr; i++) {
          char pathname[256];
          snprintf(pathname, 256, "%s/%s", dynampaths[i], dynamargs[0]);
          if (!access(pathname, X_OK)) {
            free(dynamargs[0]);
            dynamargs[0] = strdup(pathname);
            in_path = true;
            break;
          }
        }
        // INFO: If the cmd arg is not in any paths:
        // throw error and proceed to the shell prompt.
        if (!in_path) {
          print_err_msg();
          dynamargs = cleanup_pp_dynamarray(dynamargs);
          free(redirect_path);
          continue;
        }

        pid_t rc = fork();
        if (rc == -1) {
          perror("fork() failed");
          dynamargs = cleanup_pp_dynamarray(dynamargs);
          free(redirect_path);
          dynamcmds = cleanup_pp_dynamarray(dynamcmds);
          dynampaths = cleanup_pp_dynamarray(dynampaths);
          free(line);
          exit(EXIT_FAILURE);

        } else if (rc == 0) { // Child process
          // ------------------------------------------------------------
          // Implement redirection
          if (redirected) {
            int fd = open(redirect_path, O_WRONLY | O_CREAT | O_TRUNC,
                          S_IRUSR | S_IWUSR | S_IROTH | S_IRGRP);
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
          }
          // ------------------------------------------------------------
          execv(dynamargs[0], dynamargs);
          // We free the memory we allocated only if execv() fails.
          // All elements but the last (char *)NULL where allocated with
          // strdup()
          perror("execv() failed to execute");
          dynamargs = cleanup_pp_dynamarray(dynamargs);
          free(redirect_path);
          dynamcmds = cleanup_pp_dynamarray(dynamcmds);
          dynampaths = cleanup_pp_dynamarray(dynampaths);
          free(line);
          exit(EXIT_FAILURE);
          // ------------------------------------------------------------
        } else { // Parent process
          pid_t wpid = waitpid(rc, NULL, 0);
          // DEBUG
          assert(rc == wpid);
        }
      } // if(dynampaths)

      dynamargs = cleanup_pp_dynamarray(dynamargs);
      free(redirect_path);
    } // for each dynam_parllcmds[i];

    // After all processes are done return control to the user through
    // the shell prompt or if in batch mode proceed to the next line if any.
    dynamcmds = cleanup_pp_dynamarray(dynamcmds);

  } // infinite shell loop

  dynampaths = cleanup_pp_dynamarray(dynampaths);
  free(line);
  if (batch_mode) {
    fclose(input_fp);
  } else {
    puts("");
  }

  return EXIT_SUCCESS;
}
