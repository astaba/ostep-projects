/* ostep-projects/processes-shell/wish.c */
// Created on:   Thu Sep 18 15:14:14 +01 2025
// Completed on: Tue Sep 30 18:38:56 +01 2025
// INFO: Usage: This 'wish' shell default path holds "/bin" directory.
// Without custom path only three builtins commands are available:
// path, cd, exit.
// Any call to the path cmd should either clear the custom path:
// wish> path
// or overwrite it:
// wish> path /usr/bin /home/owner/bin

#include <assert.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <signal.h>
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

int trim_whitespaces(char **token) {
  *token += strspn(*token, " \t");
  int len = strlen(*token);
  char *end = *token + len - 1;
  while (end > *token && (*end == ' ' || *end == '\t')) {
    *end = '\0';
    end--;
  }

  if (!*(*token))
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}

int main(int argc, char *argv[argc + 1]) {
  // INFO: If the shell is invoked with more than one file:
  // throw exception and exit(1)
  if (argc > 2) {
    print_err_msg();
    exit(EXIT_FAILURE);
  }

  bool batch_mode = false;
  FILE *input_fp = stdin;

  if (argv[1]) {
    input_fp = fopen(argv[1], "r");
    if (!input_fp) {
      print_err_msg();
      exit(EXIT_FAILURE);
    }
    batch_mode = true;
  }

  char *line = NULL;
  size_t len = 0;
  char **dynampaths_pp = NULL;

  // Set initial default path to "/bin"
  if (!(dynampaths_pp = calloc(2, sizeof(char *)))) {
    perror("calloc() failed");
    if (batch_mode)
      fclose(input_fp);

    exit(EXIT_FAILURE);
  }
  dynampaths_pp[0] = strdup("/bin");
  dynampaths_pp[1] = NULL;

  signal(SIGINT, SIG_IGN);

  while (1) {
    if (!batch_mode) {
      printf("wish> "); // Display prompt
    }

    ssize_t nread = getline(&line, &len, input_fp);
    if (nread == -1) { // Terminate wish shell
      break;
    }

    line[strcspn(line, "\n")] = '\0'; // Make the input a C standard string

    // WARNING: Make sure to perform memory cleanup required by the scope
    // before jumping to next command(s).

    char *lineptr_cpy = line;
    char *cmd_token = NULL;
    // Right after reading one input line search parallel commands (&).
    char **dynamcmds_pp = NULL; // The commands table
    // The number of cmds in the cmds table ignoring the terminating NULL
    size_t cmds_nbr = 0;

    while ((cmd_token = strsep(&lineptr_cpy, "&"))) {
      // Advance the token pointer passed any leading whitespaces
      // Then check for empty cmds and dump them before jumping the next cmd.
      if (trim_whitespaces(&cmd_token) == EXIT_FAILURE) {
        continue;
      }

      char *tmp = realloc(dynamcmds_pp, sizeof(char *) * (cmds_nbr + 2));
      if (!tmp) {
        perror("realloc() failed");
        dynamcmds_pp = cleanup_pp_dynamarray(dynamcmds_pp);
        free(line);
        exit(EXIT_FAILURE);
      } else {
        dynamcmds_pp = (char **)tmp;
      }

      dynamcmds_pp[cmds_nbr] = strdup(cmd_token);
      if (!dynamcmds_pp[cmds_nbr]) {
        perror("strdup() failed");
        dynamcmds_pp = cleanup_pp_dynamarray(dynamcmds_pp);
        free(line);
        exit(EXIT_FAILURE);
      }
      cmds_nbr++;
      dynamcmds_pp[cmds_nbr] = NULL;
    }

    if (!dynamcmds_pp)
      continue;
    // WARNING: Confirm cmds table exists before entering next loop

    // Implement paralell commands logic premices.
    // Declare a process id set to collect processes ids.
    pid_t *pids_p = calloc(cmds_nbr, sizeof(pid_t *));
    if (!pids_p) {
      perror("calloc() failed");
      dynamcmds_pp = cleanup_pp_dynamarray(dynamcmds_pp);
      free(line);
      exit(EXIT_FAILURE);
    }

    for (size_t i = 0; dynamcmds_pp[i]; i++) {
      // Count the number of '>' characters in the input cmd and turn on the
      // redirected flag if any.
      bool redirected = false;
      size_t chevron_count = ({
        char *s = dynamcmds_pp[i];
        size_t n = 0;
        while ((s = strchr(s, '>'))) {
          n++;
          s++;
        }
        n;
      });
      // In case of formatting error involving multiple '>' characters in the
      // input cmd ignore the cmd and jump to next cmd.
      if (chevron_count > 1 || strspn(dynamcmds_pp[i], ">")) {
        print_err_msg();
        continue; // Skip this cmd and go to next cmd in the cmds table
      } else if (chevron_count) {
        redirected = true;
      }

      char *cmdptr_cpy = dynamcmds_pp[i];
      char *arg_token = NULL;
      char *sub_token = NULL;
      // Numbers of elements (args) from redirection '>' (including) onwards.
      size_t chevron_onwards = 0;
      // Parse out actual cmd and optional arguments within an input cmd.
      char **dynamargs_pp =
          NULL; // Table of cmd elements: cmd [args ...] [> file]
      // Number of elements in the arguments table ignoring the terminating NULL
      size_t args_nbr = 0;
      // Hypothetic unique argument following the redirection '>'
      char *redirect_path = NULL;

      while ((arg_token = strsep(&cmdptr_cpy, "\t "))) {
        // Ignore consecutive whitespaces within commands.
        if (!*arg_token)
          continue;

        while ((sub_token = strsep(&arg_token, ">"))) {

          if (!chevron_onwards) {
            if (arg_token) {
              chevron_onwards = 1;
            }

            char *tmp = realloc(dynamargs_pp, sizeof(char *) * (args_nbr + 2));
            if (!tmp) {
              perror("realloc() failed");
              dynamargs_pp = cleanup_pp_dynamarray(dynamargs_pp);
              dynamcmds_pp = cleanup_pp_dynamarray(dynamcmds_pp);
              free(pids_p);
              free(line);
              exit(EXIT_FAILURE);
            } else {
              dynamargs_pp = (char **)tmp;
            }
            dynamargs_pp[args_nbr] = strdup(sub_token);
            if (!dynamargs_pp[args_nbr]) {
              perror("strdup() failed");
              dynamargs_pp = cleanup_pp_dynamarray(dynamargs_pp);
              dynamcmds_pp = cleanup_pp_dynamarray(dynamcmds_pp);
              free(pids_p);
              free(line);
              exit(EXIT_FAILURE);
            }
            args_nbr++;
            dynamargs_pp[args_nbr] = NULL;
          } else if (*sub_token) {
            if (chevron_onwards == 1) {
              redirect_path = strdup(sub_token);
              if (!redirect_path) {
                perror("strdup() failed");
                dynamargs_pp = cleanup_pp_dynamarray(dynamargs_pp);
                dynamcmds_pp = cleanup_pp_dynamarray(dynamcmds_pp);
                free(pids_p);
                free(line);
                exit(EXIT_FAILURE);
              }
            }
            chevron_onwards++;
          }
        }
      }

      // In case of formatting error involving either no argument or more than
      // one argument after the redirection input or trying to redirect a
      // builtin command, cmd cleanup the cmd and jump to next cmd.
      if (redirected) {
        bool is_builtin = !strcmp(dynamargs_pp[0], "path") ||
                          !strcmp(dynamargs_pp[0], "cd") ||
                          !strcmp(dynamargs_pp[0], "exit");
        if (chevron_onwards == 1 || chevron_onwards > 2 || is_builtin) {
          print_err_msg();
          dynamargs_pp = cleanup_pp_dynamarray(dynamargs_pp);
          free(redirect_path);
          continue; // Skip this cmd and jump to next cmd in cmds table
        }
      }

      if (!redirected) {
        // INFO: The shell has its own builtins cmd implemented through
        // syscalls: path, cd and exit.
        if (!strcmp(dynamargs_pp[0], "exit")) {
          if (args_nbr > 1) {
            print_err_msg();
            dynamargs_pp = cleanup_pp_dynamarray(dynamargs_pp);
            continue; // Jump to next parallel cmd.
          } else {
            // WARNING: Cleanup dynampaths ???
            dynamargs_pp = cleanup_pp_dynamarray(dynamargs_pp);
            dynamcmds_pp = cleanup_pp_dynamarray(dynamcmds_pp);
            free(pids_p);
            free(line);
            dynampaths_pp = cleanup_pp_dynamarray(dynampaths_pp);
            if (batch_mode)
              fclose(input_fp);
            exit(EXIT_SUCCESS);
          }
        } else if (!strcmp(dynamargs_pp[0], "cd")) {
          if (args_nbr != 2) {
            print_err_msg();
          } else {
            if (chdir(dynamargs_pp[1]) == -1) {
              perror("wish cd");
            }
          }
          dynamargs_pp = cleanup_pp_dynamarray(dynamargs_pp);
          continue; // Jump to next parallel cmd.

        } else if (!strcmp(dynamargs_pp[0], "path")) {
          if (args_nbr < 2) {
            // Clearing the path with the simple: wish> path
            dynampaths_pp = cleanup_pp_dynamarray(dynampaths_pp);
          } else {
            dynampaths_pp = cleanup_pp_dynamarray(dynampaths_pp);
            dynampaths_pp = calloc(args_nbr, sizeof(char *));
            if (!dynampaths_pp) {
              perror("calloc() failed");
              dynamargs_pp = cleanup_pp_dynamarray(dynamargs_pp);
              dynamcmds_pp = cleanup_pp_dynamarray(dynamcmds_pp);
              free(pids_p);
              free(line);
              // WARNING: In batch mode cleanup input_fp before exit
              exit(EXIT_FAILURE);
            }
            for (size_t j = 0; j < (args_nbr - 1); j++) {
              dynampaths_pp[j] = strdup(dynamargs_pp[j + 1]);
              if (!dynampaths_pp[j]) {
                perror("strdup failed");
                dynamargs_pp = cleanup_pp_dynamarray(dynamargs_pp);
                dynamcmds_pp = cleanup_pp_dynamarray(dynamcmds_pp);
                free(pids_p);
                dynampaths_pp = cleanup_pp_dynamarray(dynampaths_pp);
                free(line);
                exit(EXIT_FAILURE);
              }
            }
            dynampaths_pp[args_nbr - 1] = NULL;
          }
          dynamargs_pp = cleanup_pp_dynamarray(dynamargs_pp);
          continue; // Jump to next parallel cmd.
        } // if cmd0 == path
      } // if not redirected

      // Check dynamargs[0] exists in one of the path directories
      bool in_path = false;
      for (size_t i = 0; dynampaths_pp && dynampaths_pp[i]; i++) {
        char pathname[PATH_MAX];
        snprintf(pathname, PATH_MAX, "%s/%s", dynampaths_pp[i],
                 dynamargs_pp[0]);
        if (!access(pathname, X_OK)) {
          free(dynamargs_pp[0]);
          dynamargs_pp[0] = strdup(pathname);
          in_path = true;
          break;
        }
      }
      // If the cmd arg not in paths: throw error and proceed to shell prompt.
      if (!in_path) {
        print_err_msg();
        dynamargs_pp = cleanup_pp_dynamarray(dynamargs_pp);
        free(redirect_path);
        continue;
      }

      pid_t rc = fork();
      if (rc == -1) {
        perror("fork() failed");
        dynamargs_pp = cleanup_pp_dynamarray(dynamargs_pp);
        free(redirect_path);
        dynamcmds_pp = cleanup_pp_dynamarray(dynamcmds_pp);
        free(pids_p);
        dynampaths_pp = cleanup_pp_dynamarray(dynampaths_pp);
        free(line);
        if (batch_mode)
          fclose(input_fp);
        exit(EXIT_FAILURE);

      } else if (rc == 0) { // Child process
        // Implement redirection
        if (redirected) {
          int fd = open(redirect_path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC,
                        S_IRUSR | S_IWUSR | S_IROTH | S_IRGRP);
          if (fd == -1) {
            perror("open redirect failed");
            dynamargs_pp = cleanup_pp_dynamarray(dynamargs_pp);
            free(redirect_path);
            dynamcmds_pp = cleanup_pp_dynamarray(dynamcmds_pp);
            free(pids_p);
            dynampaths_pp = cleanup_pp_dynamarray(dynampaths_pp);
            free(line);
            if (batch_mode)
              fclose(input_fp);
            exit(EXIT_FAILURE);
          }

          if (dup2(fd, STDOUT_FILENO) == -1 || dup2(fd, STDERR_FILENO) == -1) {
            perror("dup2 failed");
            dynamargs_pp = cleanup_pp_dynamarray(dynamargs_pp);
            free(redirect_path);
            dynamcmds_pp = cleanup_pp_dynamarray(dynamcmds_pp);
            free(pids_p);
            dynampaths_pp = cleanup_pp_dynamarray(dynampaths_pp);
            free(line);
            if (batch_mode)
              fclose(input_fp);
            exit(EXIT_FAILURE);
          }
          close(fd);
        }
        // ------------------------------------------------------------
        // Restore (Ctrl-C)
        signal(SIGINT, SIG_DFL);
        // ------------------------------------------------------------
        execv(dynamargs_pp[0], dynamargs_pp);
        // We free the memory we allocated only if execv() fails.
        // All elements but the last (char *)NULL where allocated with
        // strdup()
        perror("execv() failed to execute");
        dynamargs_pp = cleanup_pp_dynamarray(dynamargs_pp);
        free(redirect_path);
        dynamcmds_pp = cleanup_pp_dynamarray(dynamcmds_pp);
        free(pids_p);
        dynampaths_pp = cleanup_pp_dynamarray(dynampaths_pp);
        free(line);
        _exit(EXIT_FAILURE);
        // ------------------------------------------------------------
      } else { // Parent process
        // Collect processes ids and let the for(dynamcmds_pp[i])loop run
        // parallel commands in parallel.
        pids_p[i] = rc;
      }

      dynamargs_pp = cleanup_pp_dynamarray(dynamargs_pp);
      free(redirect_path);
    } // for each cmd in the cmds table

    // Wait for all parallel commands to complete
    for (size_t i = 0; i < cmds_nbr; i++) {
      if (pids_p[i] > 0)
        waitpid(pids_p[i], NULL, 0);
    }
    free(pids_p);
    // After all processes are done return control to the user through
    // the shell prompt or if in batch mode proceed to the next line if any.
    dynamcmds_pp = cleanup_pp_dynamarray(dynamcmds_pp);
  } // infinite shell loop

  dynampaths_pp = cleanup_pp_dynamarray(dynampaths_pp);
  free(line);
  if (batch_mode) {
    fclose(input_fp);
  } else {
    puts("");
  }

  return EXIT_SUCCESS;
}
