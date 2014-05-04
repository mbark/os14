/* minishell is a simple shell. It will continually prompt for user input and
   interpret the input as commands to be run. Commands cannot be longer than 70
   characters and commands cannot have more than 5 arguments.

   minishell can run commands as foreground or background processes. When a
   command is run as a foreground process the shell will wait until the command
   finishes before continuing execution. The shell prints the running time of
   foreground processes in milliseconds. If the last argument given to the
   command is & then the command will be run as a background process. When a
   command is run as a background process the shell will prompt for input
   immediately without waiting for the process to end. The shell will print a
   note when it detects that a background process has ended.

   The shell has two built-in commands, cd and exit. cd changes the working
   directory to the directory given as the first argument. If the directory does
   not exist cd will change the directory to the HOME directory. exit kills all
   child processes of the shell and then exits the shell. */
#define _POSIX_SOURCE
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>

/* The maximum allowed length of a command. */
#define MAX_COMMAND_LENGTH (70)
/* The maximum allowed number of arguments given to a command. */
#define MAX_NUM_ARGS (5)

void handleSignal(int);
void readCommand(char*, int);
void parseCommand(char*, char**, char**);
void cd(char*);
void goHome();
void quit(int);
void runCommand(char*, char*[]);
unsigned long getTimestamp();

/* The start point of the program. The main function contains the main loop and
   structure of the program. The main loop prompts the user for input, parses
   the input into a command and arguments and executes the command. It also
   contains the logic for handling the built-in commands.
   argc is the number of command line arguments given to minishell.
   argv contains the command line arguments given to minishell. These arguments
   are not used. */
int main(int argc, char* argv[]) {
  /* Buffer that holds the input from the user. */
  char buffer[MAX_COMMAND_LENGTH];
  /* Holds the string that is the command name part of the user input. */
  char* command;
  /* Holds the strings that are the arguments part of the user input. We make
     the size of the array 2 larger than the maximum number of arguments since
     the first argument needs to be the name of the command and the array has
     to be null-terminated. */
  char* args[MAX_NUM_ARGS + 2];
  /* Holds the return value of the waitpid call. */
  pid_t child_pid;
  args[MAX_NUM_ARGS + 1] = NULL;

  handleSignal(SIGINT);
  handleSignal(SIGTERM);

  while(1) {
    /* If waitpid returns a value > 0 then a process has terminated.
       If the return value is 0 then there are children but none of them
       has terminated.
       If the return value is < 0 then an error has occurred, but the only
       possible error when the WNOHANG flag is specified is that there are
       no children, in which case there are no background processes. */
    while((child_pid = waitpid(-1, NULL, WNOHANG)) > 0) {
      printf("Background process %d terminated\n", child_pid);
    }

    printf("minishell>");
    readCommand(buffer, MAX_COMMAND_LENGTH);
    parseCommand(buffer, &command, args);
    if(command == NULL) {
      continue;
    }

    if(strcmp(command, "exit") == 0) {
      quit(0);
    } else if(strcmp(command, "cd") == 0) {
      cd(args[1]);
    } else {
      runCommand(command, args);
    }
  }

  exit(1);
}

/* Installs a signal handler that ignores the given signal number.
   If the signal handler cannot be installed the shell exits.
   sig is the signal number to ignore. */
void handleSignal(int sig) {
  if(signal(sig, SIG_IGN) == SIG_ERR) {
    printf("Failed to install signal handler for signal %d\n", sig);
    exit(1);
  }
}

/* Reads a line of user input. The input is stored in the given buffer.
   A maximum of max_size characters will be read. If an error occurs
   while reading or if end of file is encountered the shell exits.
*/
void readCommand(char* buffer, int max_size) {
  /* holds the length of the string read in the buffer. */
  size_t ln;
  /* holds the return value of the fgets call. Used for error detection. */
  char* return_value = fgets(buffer, max_size, stdin);
  /* fgets returns NULL when EOF occurs while no characters have been read
     or on error. In the first case no more characters can be read and we exit.
     In the second case we exit because it cannot be known if more characters
     can be read. */
  if(return_value == NULL) {
    quit(1);
  }

  /* Removes the newline character at the end of the buffer if it is
     present. */
  ln = strlen(buffer) - 1;
  if(buffer[ln] == '\n') {
    buffer[ln] = '\0';
  }
}

/* Parses the user input stored in buffer into a command name and an
   array of arguments. The command name will be stored in the command
   variable and the arguments will be stored in the args array. */
void parseCommand(char* buffer, char** command, char** args) {
  /* Holds the loop index. */
  int i;
  *command = strtok(buffer, " ");
  args[0] = *command;
  for(i = 1; i < MAX_NUM_ARGS + 1; i++) {
    args[i] = strtok(NULL, " ");
  }
}

/* The built-in cd command. Changes the working directory to the
   one given in the argument. If the given directory is invalid
   an attempt to go to the HOME directory is made. */
void cd(char* directory) {
  if(directory == NULL) {
    goHome();
    return;
  }

  if(chdir(directory) == -1) {
    fprintf(stderr, "Unable to go to directory %s. Attempting to go to HOME directory\n", directory);
    goHome();
  }
}

/* Changes the working directory to the HOME directory. If the HOME
   environment variable is not set an error message is printed. */
void goHome() {
  /* Holds the value of the home environment variable. */
  char* home = getenv("HOME");
  if(chdir(home) == -1) {
    fprintf(stderr, "Unable to go to HOME directory. Possibly not set.\n");
  }
}

/* Kills all child processes and exits with the given status. */
void quit(int status) {
  /* Send a termination signal to all children. We ignore the
     return value of the kill call, because if it fails we have
     no other way of terminating all children so we exit. */
    kill(0, SIGTERM);
    exit(status);
  }

/* Runs the given command in a child process with the given arguments. Whether
   the command is run as a foreground or background process is determined by
   inspecting the last argument. If the last argument is & then the command is
   run as a background process.
   If the command is run as a foreground process the function will wait until
   the command finishes before returning. The execution time of the command
   will be printed in milliseconds.
   If the command is run as a background process the function will return
   immediately. */
void runCommand(char* command, char* args[]) {
  /* Boolean value to determine if the process should be run as a foreground
     or background process. Determined by inspecting the last argument. */
  int background = 0;
  /* Holds the loop index. */
  int i;
  /* Holds the timestamp at the beginning of command execution. Used to
     determine the running time of foreground commands. */
  unsigned long startTime;
  /* Holds the return value of the fork system call. */
  pid_t pid = fork();
  if(pid == -1) {
    fprintf(stderr, "Unable to fork.\n");
    quit(1);
  }

  for(i = MAX_NUM_ARGS; i >= 0; i--) {
    if(args[i] != NULL) {
      if(strcmp(args[i], "&") == 0) {
        background = 1;
        args[i] = NULL;
      }
      break;
    }
  }

  if(pid == 0) {
    execvp(command, args);
    fprintf(stderr, "Unable to start command: %s\n", command);
    quit(1);
  } else if(pid > 0) {
    startTime = getTimestamp();
    printf("Started %s process with pid %d\n", background ? "background" : "foreground", pid);
    if(background) {
      return;
    }

    /* waitpid returns -1 on error. Since we know that pid is a valid pid
       the only error that can occur is EINTR, which means that we received
       an unblocked signal. In this case we continue waiting. */
    while(waitpid(pid, NULL, 0) == -1);
    printf("Foreground process %d ended\n", pid);
    printf("Wallclock time: %.3f\n", (getTimestamp() - startTime) / 1000.0);
  }
}

/* Returns the current timestamp in microseconds. Returns 0 if the current
   time could not be read. */
unsigned long getTimestamp() {
  /* Holds the current time from the gettimeofday call. */
  struct timeval tv;
  if(gettimeofday(&tv,NULL) == -1) {
    /* If we are unable to get the time of day we return 0, as this
       should not be a fatal error. */
    fprintf(stderr, "Unable to get current time stamp.\n");
    return 0;
  }
  return 1000000 * tv.tv_sec + tv.tv_usec;
}
