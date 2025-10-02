/* ostep-projects/processes-shell/wish-v2.c */
// Created on: Tue Sep 30 18:42:41 +01 2025
// Completed on: Thu Oct  2 22:58:55 +01 2025
// INFO: Usage: This 'wish' shell default path holds "/bin" directory.
// Without custom path only three builtins commands are available:
// path, cd, exit.
// Any call to the path cmd should either clear the custom path:
// wish> path
// or overwrite it:
// wish> path /usr/bin /home/owner/bin

#include <assert.h>
#include <ctype.h>
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

typedef enum {
  BUILTIN_NONE = 0,
  BUILTIN_PATH,
  BUILTIN_CD,
  BUILTIN_EXIT
} builtins_t;

char **free_dupstring_array(char **arr) {
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
  while (end > *token && (isspace((unsigned char)*end))) {
    *end = '\0';
    end--;
  }

  if (!*(*token))
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}

static inline builtins_t detect_builtins(const char *cmd) {
  if (!strcmp(cmd, "path"))
    return BUILTIN_PATH;
  if (!strcmp(cmd, "cd"))
    return BUILTIN_CD;
  if (!strcmp(cmd, "exit"))
    return BUILTIN_EXIT;
  return BUILTIN_NONE;
}

int main(int argc, char *argv[argc + 1]) {
  if (argc > 2) {
    print_err_msg();
    exit(EXIT_FAILURE);
  }

  // Set shell mode and open input stream
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

  // Set initial default path to "/bin"
  char **dynampaths_pp = calloc(2, sizeof(char *));
  if (!dynampaths_pp) {
    perror("calloc() failed");
    if (batch_mode)
      fclose(input_fp);
    exit(EXIT_FAILURE);
  }
  dynampaths_pp[0] = strdup("/bin");
  dynampaths_pp[1] = NULL;

  // Prevent Ctrl-C from killing parent porcess
  signal(SIGINT, SIG_IGN);

  // Launch shell loop
  char *line = NULL;
  size_t len = 0;

  while (1) {
    if (!batch_mode)
      printf("wish> "); // Display prompt

    ssize_t nread = getline(&line, &len, input_fp);
    if (nread == -1) // Terminate wish shell
      break;

    line[strcspn(line, "\n")] = '\0'; // Make the input a C standard string

    // WARNING: Make sure to perform memory cleanup required by each scope
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
        goto parent_failure_0;
      } else {
        dynamcmds_pp = (char **)tmp;
      }

      dynamcmds_pp[cmds_nbr] = strdup(cmd_token);
      if (!dynamcmds_pp[cmds_nbr]) {
        perror("strdup() failed");
        goto parent_failure_0;
      }

      cmds_nbr++;
      dynamcmds_pp[cmds_nbr] = NULL;
    }
    // WARNING: Confirm cmds table exists before entering next loop
    if (!dynamcmds_pp)
      continue;

    // Implement paralell commands logic premices.
    // Declare a process id set to collect processes ids.
    pid_t *pids_p = calloc(cmds_nbr, sizeof(pid_t));
    if (!pids_p) {
      perror("calloc() failed");
      goto parent_failure_0;
    }

    //=============  Run commands in parallel  =================================
    for (size_t i = 0; dynamcmds_pp[i]; i++) {
      // ===========  1. Count #chevron  ==================
      size_t chevron_count = 0;
      for (char *s = dynamcmds_pp[i]; (s = strchr(s, '>')); s++) {
        chevron_count++;
      }
      if (chevron_count > 1) {
        print_err_msg();
        continue; // Skip this cmd and go to next cmd in the cmds table
      }
      // ===========  2. Split at chevron  ================
      char *cmd_lhs = dynamcmds_pp[i];
      char *cmd_rhs = NULL;
      if (chevron_count == 1) {
        cmd_rhs = strchr(cmd_lhs, '>');
        *cmd_rhs++ = '\0';
      }
      // ===========  3. Tokenize cmd_lhs into arguments  ================
      // TODO: quoted arguments tokenization.
      char **dynamargs_pp = NULL;
      char *arg_token = NULL;
      size_t args_nbr = 0;

      while ((arg_token = strsep(&cmd_lhs, " \t"))) {
        if (!*arg_token) // Ignore consecutive whitespaces
          continue;

        char *tmp = realloc(dynamargs_pp, sizeof(char *) * (args_nbr + 2));
        if (!tmp) {
          perror("realloc() failed");
          goto parent_failure_1;
        } else {
          dynamargs_pp = (char **)tmp;
        }

        dynamargs_pp[args_nbr] = strdup(arg_token);
        if (!dynamargs_pp[args_nbr]) {
          perror("strdup() failed");
          goto parent_failure_1;
        }

        args_nbr++;
        dynamargs_pp[args_nbr] = NULL;
      }

      char *redirect_path = NULL;
      bool bad_command_format = false;
      do {
        if (!dynamargs_pp) { // empty command or no command before '>'
          bad_command_format = true;
          break;
        }
        // ===========  4. Handle the redirection  =========================
        if (cmd_rhs) {
          if (trim_whitespaces(&cmd_rhs) == EXIT_FAILURE) { // no path after '>'
            bad_command_format = true;
            break;
          }
          char *extra_arg = NULL; // More than one argument after '>'
          if ((extra_arg = strpbrk(cmd_rhs, " \t")) && *extra_arg) {
            bad_command_format = true;
            break;
          }

          redirect_path = strdup(cmd_rhs);
          if (!redirect_path) {
            perror("strdup() failed");
            goto parent_failure_1;
          }
        }
      } while (false);
      if (bad_command_format) {
        print_err_msg();
        dynamargs_pp = free_dupstring_array(dynamargs_pp);
        continue; // Skip this cmd and jump to next cmd in cmds table
      }

      builtins_t which_builtin = detect_builtins(dynamargs_pp[0]);

      if ((which_builtin != BUILTIN_NONE) && redirect_path) {
        print_err_msg();
        dynamargs_pp = free_dupstring_array(dynamargs_pp);
        free(redirect_path);
        continue; // Skip this cmd and jump to next cmd in cmds table
      }

      if (which_builtin == BUILTIN_PATH) {
        if (args_nbr < 2) {
          // Clearing the path with the simple: wish> path
          dynampaths_pp = free_dupstring_array(dynampaths_pp);
        } else {
          dynampaths_pp = free_dupstring_array(dynampaths_pp);

          dynampaths_pp = calloc(args_nbr, sizeof(char *));
          if (!dynampaths_pp) {
            perror("calloc() failed");
            goto parent_failure_1;
          }

          for (size_t j = 0; j < (args_nbr - 1); j++) {
            dynampaths_pp[j] = strdup(dynamargs_pp[j + 1]);
            if (!dynampaths_pp[j]) {
              perror("strdup failed");
              goto parent_failure_1;
            }
            dynampaths_pp[j + 1] = NULL;
          }
        }

        dynamargs_pp = free_dupstring_array(dynamargs_pp);
        continue; // Jump to next parallel cmd.
      }

      if (which_builtin == BUILTIN_CD) {
        if (args_nbr != 2) {
          print_err_msg();
        } else {
          if (chdir(dynamargs_pp[1]) == -1) {
            perror("wish cd");
          }
        }
        dynamargs_pp = free_dupstring_array(dynamargs_pp);
        continue; // Jump to next parallel cmd.
      }

      if (which_builtin == BUILTIN_EXIT) {
        if (args_nbr > 1) {
          print_err_msg();
          dynamargs_pp = free_dupstring_array(dynamargs_pp);
          continue; // Jump to next parallel cmd.
        } else {
          dynamargs_pp = free_dupstring_array(dynamargs_pp);
          free(pids_p);
          dynamcmds_pp = free_dupstring_array(dynamcmds_pp);
          dynampaths_pp = free_dupstring_array(dynampaths_pp);
          free(line);
          if (batch_mode)
            fclose(input_fp);
          exit(EXIT_SUCCESS);
        }
      }

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
        dynamargs_pp = free_dupstring_array(dynamargs_pp);
        free(redirect_path);
        continue;
      }

      pid_t rc = fork();
      if (rc == -1) {
        perror("fork() failed");
        free(redirect_path);
      parent_failure_1:
        dynamargs_pp = free_dupstring_array(dynamargs_pp);
        free(pids_p);
      parent_failure_0:
        dynamcmds_pp = free_dupstring_array(dynamcmds_pp);
        dynampaths_pp = free_dupstring_array(dynampaths_pp);
        free(line);
        if (batch_mode)
          fclose(input_fp);
        exit(EXIT_FAILURE);

      } else if (rc == 0) {
        // =========================== Child process  ==========================
        // Implement redirection
        if (redirect_path) {

          int fd = open(redirect_path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC,
                        S_IRUSR | S_IWUSR | S_IROTH | S_IRGRP);
          if (fd == -1) {
            perror("open redirect failed");
            goto child_failure;
          }

          if (dup2(fd, STDOUT_FILENO) == -1 || dup2(fd, STDERR_FILENO) == -1) {
            perror("dup2 failed");
            goto child_failure;
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
      child_failure:
        free(redirect_path);
        dynamargs_pp = free_dupstring_array(dynamargs_pp);
        free(pids_p);
        dynamcmds_pp = free_dupstring_array(dynamcmds_pp);
        dynampaths_pp = free_dupstring_array(dynampaths_pp);
        free(line);
        if (batch_mode)
          fclose(input_fp);
        _exit(EXIT_FAILURE);
        // ------------------------------------------------------------
      }

      // =================  Parent process  ====================================
      // Collect processes ids and let the for(dynamcmds_pp[i])loop run
      // parallel commands in parallel.
      pids_p[i] = rc;
      dynamargs_pp = free_dupstring_array(dynamargs_pp);
      free(redirect_path);
    } // end of single parallel cmd

    // ================  Before going back to prompt  ==========================
    // Wait for all parallel commands to complete
    for (size_t i = 0; i < cmds_nbr; i++) {
      if (pids_p[i] > 0)
        waitpid(pids_p[i], NULL, 0);
    }
    free(pids_p);
    // After all processes are done return control to the user through
    // the shell prompt or if in batch mode proceed to the next line if any.
    dynamcmds_pp = free_dupstring_array(dynamcmds_pp);

  } // end infinite shell loop

  dynampaths_pp = free_dupstring_array(dynampaths_pp);
  free(line);
  if (batch_mode) {
    fclose(input_fp);
  } else {
    puts("");
  }

  return EXIT_SUCCESS;
}
