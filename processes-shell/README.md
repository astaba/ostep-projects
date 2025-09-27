
# Unix Shell

In this project, you'll build a simple Unix shell. The shell is the heart of
the command-line interface, and thus is central to the Unix/C programming
environment. Mastering use of the shell is necessary to become proficient in
this world; knowing how the shell itself is built is the focus of this
project.

There are three specific objectives to this assignment:

* To further familiarize yourself with the Linux programming environment.
* To learn how processes are created, destroyed, and managed.
* To gain exposure to the necessary functionality in shells.

## Overview

In this assignment, you will implement a *command line interpreter (CLI)* or,
as it is more commonly known, a *shell*. The shell should operate in this
basic way: when you type in a command (in response to its prompt), the shell
creates a child process that executes the command you entered and then prompts
for more user input when it has finished.

The shells you implement will be similar to, but simpler than, the one you run
every day in Unix. If you don't know what shell you are running, it's probably
`bash`. One thing you should do on your own time is learn more about your
shell, by reading the man pages or other online materials.

## Program Specifications

### Basic Shell: `wish`

Your basic shell, called `wish` (short for Wisconsin Shell, naturally), is
basically an interactive loop: it repeatedly prints a prompt `wish> ` (note
the space after the greater-than sign), parses the input, executes the command
specified on that line of input, and waits for the command to finish. This is
<!-- TODO: DONE -->
repeated until the user types `exit`.  The name of your final executable
should be `wish`.

The shell can be invoked with either no arguments or a single argument;
anything else is an error. Here is the no-argument way:

```
prompt> ./wish
wish>
```

At this point, `wish` is running, and ready to accept commands. Type away!

The mode above is called *interactive* mode, and allows the user to type
commands directly. The shell also supports a *batch mode*, which instead reads
<!-- TODO: -->
input from a batch file and executes commands from therein. Here is how you
run the shell with a batch file named `batch.txt`:

```
prompt> ./wish batch.txt
```

One difference between batch and interactive modes: in interactive mode, a
prompt is printed (`wish> `). In batch mode, no prompt should be printed.

You should structure your shell such that it creates a process for each new
command (the exception are *built-in commands*, discussed below).  Your basic
shell should be able to parse a command and run the program corresponding to
<!-- TODO: DONE -->
the command.  For example, if the user types `ls -la /tmp`, your shell should
run the program `/bin/ls` with the given arguments `-la` and `/tmp` (how does
the shell know to run `/bin/ls`? It's something called the shell **path**;
more on this below).

## Structure

### Basic Shell

The shell is very simple (conceptually): it runs in a while loop, repeatedly
asking for input to tell it what command to execute. It then executes that
<!-- TODO: DONE -->
command. The loop continues indefinitely, until the user types the built-in
command `exit`, at which point it exits. That's it!

For reading lines of input, you should use `getline()`. This allows you to
obtain arbitrarily long input lines with ease. Generally, the shell will be
run in *interactive mode*, where the user types a command (one at a time) and
the shell acts on it. However, your shell will also support *batch mode*, in
which the shell is given an input file of commands; in this case, the shell
should not read user input (from `stdin`) but rather from this file to get the
commands to execute.

<!-- TODO: DONE -->
In either mode, if you hit the end-of-file marker (EOF), you should call
`exit(0)` and exit gracefully.

To parse the input line into constituent pieces, you might want to use
`strsep()`. Read the man page (carefully) for more details.

To execute commands, look into `fork()`, `exec()`, and `wait()/waitpid()`.
See the man pages for these functions, and also read the relevant [book
chapter](http://www.ostep.org/cpu-api.pdf) for a brief overview.

You will note that there are a variety of commands in the `exec` family; for
this project, you must use `execv`. You should **not** use the `system()`
library function call to run a command.  Remember that if `execv()` is
successful, it will not return; if it does return, there was an error (e.g.,
the command does not exist). The most challenging part is getting the
arguments correctly specified.

### Paths

In our example above, the user typed `ls` but the shell knew to execute the
program `/bin/ls`. How does your shell know this?

It turns out that the user must specify a **path** variable to describe the
set of directories to search for executables; the set of directories that
comprise the path are sometimes called the *search path* of the shell. The
path variable contains the list of all directories to search, in order, when
the user types a command.

**Important:** Note that the shell itself does not *implement* `ls` or other
commands (except built-ins). All it does is find those executables in one of
the directories specified by `path` and create a new process to run them.

To check if a particular file exists in a directory and is executable,
<!-- TODO: DONE -->
consider the `access()` system call. For example, when the user types `ls`,
and path is set to include both `/bin` and `/usr/bin`, try `access("/bin/ls",
X_OK)`. If that fails, try "/usr/bin/ls". If that fails too, it is an error.

<!-- TODO: -->
Your initial shell path should contain one directory: `/bin`

Note: Most shells allow you to specify a binary specifically without using a
search path, using either **absolute paths** or **relative paths**. For
example, a user could type the **absolute path** `/bin/ls` and execute the
`ls` binary without a search path being needed. A user could also specify a
**relative path** which starts with the current working directory and
specifies the executable directly, e.g., `./main`. In this project, you **do
not** have to worry about these features.

### Built-in Commands

<!-- TODO: DONE -->
Whenever your shell accepts a command, it should check whether the command is
a **built-in command** or not **(built-ins to your custom `wish` shell, not to
the standard `bash`).** If it is, it should not be executed like other
programs. Instead, your shell will invoke your implementation of the built-in
command. For example, to implement the `exit` built-in command, you simply
call `exit(0);` in your wish source code, which then will exit the shell.

In this project, you should implement `exit`, `cd`, and `path` as built-in
commands.

* `exit`: When the user types `exit`, your shell should simply call the `exit`
  system call with 0 as a parameter. It is an error to pass any arguments to
  `exit`.

* `cd`: `cd` always take one argument (0 or >1 args should be signaled as an
error). To change directories, use the `chdir()` system call with the argument
supplied by the user; if `chdir` fails, that is also an error.

* `path`: The `path` command takes 0 or more arguments, with each argument
  separated by whitespace from the others. A typical usage would be like this:
  `wish> path /bin /usr/bin`, which would add `/bin` and `/usr/bin` to the
<!-- TODO: -->
  search path of the shell. If the user sets path to be empty, then the shell
  should not be able to run any programs (except built-in commands). The
  `path` command always overwrites the old path with the newly specified
  path.

### Redirection

Many times, a shell user prefers to send the output of a program to a file
rather than to the screen. Usually, a shell provides this nice feature with
the `>` character. Formally this is named as redirection of standard
<!-- TODO: -->
output. To make your shell users happy, your shell should also include this
feature, but with a slight twist (explained below).

For example, if a user types `ls -la /tmp > output`, nothing should be printed
on the screen. Instead, the standard output of the `ls` program should be
rerouted to the file `output`. In addition, the standard error output of
the program should be rerouted to the file `output` (the twist is that this
is a little different than standard redirection).

If the `output` file exists before you run your program, you should simple
overwrite it (after truncating it).

The exact format of redirection is a command (and possibly some arguments)
followed by the redirection symbol followed by a filename. Multiple
<!-- TODO: -->
redirection operators or multiple files to the right of the redirection sign
are errors.

Note: don't worry about redirection for built-in commands (e.g., we will
not test what happens when you type `path /bin > file`).

### Parallel Commands

Your shell will also allow the user to launch parallel commands. This is
accomplished with the ampersand operator as follows:

```
wish> cmd1 & cmd2 args1 args2 & cmd3 args1
```

In this case, instead of running `cmd1` and then waiting for it to finish,
your shell should run `cmd1`, `cmd2`, and `cmd3` (each with whatever arguments
the user has passed to it) in parallel, *before* waiting for any of them to
complete.

<!-- TODO: -->
Then, after starting all such processes, you must make sure to use `wait()`
(or `waitpid`) to wait for them to complete. After all processes are done,
<!-- INFO: Batch file format -->
return control to the user as usual (or, if in batch mode, move on to the next
line).


### Program Errors

**The one and only error message.** You should print this one and only error
message whenever you encounter an error of any type:

```
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
```

The error message should be printed to stderr (standard error), as shown
above.

After most errors, your shell simply *continue processing* after
<!-- TODO: -->
printing the one and only error message. However, if the shell is
invoked with more than one file, or if the shell is passed a bad batch
file, it should exit by calling `exit(1)`.

There is a difference between errors that your shell catches and those that
the program catches. Your shell should catch all the syntax errors specified
in this project page. If the syntax of the command looks perfect, you simply
run the specified program. If there are any program-related errors (e.g.,
invalid arguments to `ls` when you run it, for example), the shell does not
have to worry about that (rather, the program will print its own error
messages and exit).


### Miscellaneous Hints

Remember to get the **basic functionality** of your shell working before
worrying about all of the error conditions and end cases. For example, first
get a single command running (probably first a command with no arguments, such
as `ls`).

Next, add built-in commands. Then, try working on redirection. Finally, think
about parallel commands. Each of these requires a little more effort on
parsing, but each should not be too hard to implement.

At some point, you should make sure your code is robust to white space of
various kinds, including spaces (` `) and tabs (`\t`). In general, the user
should be able to put variable amounts of white space before and after
commands, arguments, and various operators; however, the operators
(redirection and parallel commands) do not require whitespace.

<!-- TODO: -->
Check the return codes of all system calls from the very beginning of your
work. This will often catch errors in how you are invoking these new system
calls. It's also just good programming sense.

Beat up your own code! You are the best (and in this case, the only) tester of
this code. Throw lots of different inputs at it and make sure the shell
behaves well. Good code comes through testing; you must run many different
tests to make sure things work as desired. Don't be gentle -- other users
certainly won't be.

Finally, keep versions of your code. More advanced programmers will use a
source control system such as git. Minimally, when you get a piece of
functionality working, make a copy of your .c file (perhaps a subdirectory
with a version number, such as v1, v2, etc.). By keeping older, working
versions around, you can comfortably work on adding new functionality, safe in
the knowledge you can always go back to an older, working version if need be.

## Remaining Implementation Tasks for "wish"

ShellBased on the existing structure and TODO comments, the following features
and refinements are needed.

This is a solid start to your shell implementation! You've correctly set
up the basic structure, command parsing, and built-in handling (`exit`, `cd`),
and you've nailed the memory cleanup (`cleanup_pp_dynamarray`).

Based on your comments, here is a breakdown of the remaining work, categorized
by feature, to help you prioritize the next steps:

1. Core Command Execution & Path Resolution

    This section is critical for running external programs like `ls` or `cat`.

    - **Implement Path Search:** You need to complete the loop that iterates
    through every directory specified in the path array. For each directory,
    construct the full path (e.g., `/bin/ls`) and use the `access()` system call
    to check if the file exists and is executable (`X_OK`).
    - **Update Command Argument:** Once a valid, executable path is found,
    you must update `dynamargs[0]` to this fully qualified path (e.g., replace
    `ls` with `/bin/ls`).
    - **Final Execution:** In the child process, use `execv()` (which expects a
    full path) instead of `execvp()` (which handles path resolution internally).
    - **Error Handling:** If the command is not found in any of the defined
    paths, you must call `print_err_msg()` and continue the shell loop.
    - **`path` Built-in Refinement:** Ensure the logic correctly handles
    clearing the path (e.g., if the command is just `wish> path`). This should
    result in setting `path` to `NULL` and `path_nbr` to `0`.

2. Redirection (`>`) Implementation

    This involves specific parsing and file descriptor manipulation in the
    child process.

    - **Strict Redirection Parsing:** Before executing any external command,
    you must scan the `dynamargs` array for the single redirection operator (`>`).
    - If you find more than one `>,` it's an error.
    - If you find `>` and do not find exactly one filename immediately following
    it, it's an error.
    - **Built-in Exclusion:** Remember that built-in commands (`path`, `cd`,
    `exit`) should not handle redirection.
    - **Child Process Logic:** If redirection is valid, the child process must:
    - Open the specified output file using `open()`. Use flags that ensure
    the file is created if it doesn't exist, and truncated if it does.
    - Use `dup2()` twice: once to redirect `STDOUT_FILENO` (1) to the file
    descriptor of the new file, and once to redirect `STDERR_FILENO` (2)
    to the file descriptor of the new file.
    - Close the original file descriptor of the opened file before calling
    `execv()`.
    - **Clean Command List:** When calling `execv()`, you must pass a modified
    argument list that does not include the `>` or the filename.

3. Parallel Command Execution (`&`)

    This is the most complex parsing and synchronization task.

    - **Initial Command Separation:** The primary shell loop must first parse
    the input line by the & delimiter to get a list of independent commands
    (e.g., `cmd1 [args...] > file1`, `cmd2 [args...]`).
    - **Iterative Execution:** The parent process needs a new loop structure
    to handle this list of commands. For each command:
    - Parse it for arguments and potential redirection (`>`).
    - `fork()` a child process.
    - The child process executes the command (handling its own redirection).
    - The parent continues the loop, launching the next command without
    waiting.
    - **Parent Synchronization:** After all parallel commands have been
    launched, the parent process must iterate through the list of child
    PIDs and use `waitpid()` on each one to ensure all background tasks are
    finished before the `wish>` prompt reappears.

4. Batch Mode (`shmode_t = batch`)

    This is currently stubbed out at the beginning of main.

    - **File Reading:** You need to open the batch file and replace the
    `while(1)` interactive loop with a loop that uses `getline()` to read commands
    line-by-line from the file pointer (`fp`).
    - **Strict Error Policy:** Unlike interactive mode where errors result
    in continue, in batch mode, any command formatting error or execution
    failure must result in an error message followed by `exit(EXIT_FAILURE)`.
    - This covers all the major functional requirements remaining for
    your shell. Let me know which task you'd like to tackle first—path
    resolution and `execv()` is usually the next logical step after the
    built-ins are stable.
