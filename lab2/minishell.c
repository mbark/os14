#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>

#define MAX_COMMAND_LENGTH (70)
#define MAX_NUM_ARGS (5)

void readCommand(char*, int);
void parseCommand(char*, char**, char**);
void cd(char*);
void quit();
void runCommand(char*, char*[]);
unsigned long getTimestamp();

int main(int argc, char* argv[]) {
  char buffer[MAX_COMMAND_LENGTH];
  char* command;
  char* args[MAX_NUM_ARGS + 1];
  pid_t child_pid;

  if(signal(SIGINT, SIG_IGN) == SIG_ERR) {
    printf("Failed to install signal handler\n");
    exit(1);
  }
  
  while(1) {
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
      quit();
    } else if(strcmp(command, "cd") == 0) {
      cd(args[1]);
    } else {
      runCommand(command, args);
    }
  }
  
  exit(1);
}

void readCommand(char* buffer, int max_size) {
  size_t ln;
  fgets(buffer, max_size, stdin);
  ln = strlen(buffer) - 1;
  if(buffer[ln] == '\n') {
    buffer[ln] = '\0';
  }
}

void parseCommand(char* buffer, char** command, char** args) {
  int i;
  *command = strtok(buffer, " ");
  args[0] = *command;
  for(i = 1; i < MAX_NUM_ARGS + 1; i++) {
    args[i] = strtok(NULL, " ");
  }
}

void cd(char* directory) {
  if(directory == NULL) {
    directory = getenv("HOME");
  }

  if(chdir(directory) == -1) {
    chdir(getenv("HOME"));
  }
}

void quit() {
  exit(0);
}

void runCommand(char* command, char* args[]) {
  int background = 0;
  int i;
  pid_t pid = fork();
  unsigned long startTime;

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
    exit(1);
  } else if(pid > 0) {
    startTime = getTimestamp();
    printf("Started %s process with pid %d\n", background ? "background" : "foreground", pid);
    if(background) {
      return;
    }
    
    waitpid(pid, NULL, 0);
    printf("Foreground process %d ended\n", pid);
    printf("Wallclock time: %.3f\n", (getTimestamp() - startTime) / 1000.0);
  } else {

  }
}

unsigned long getTimestamp() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return 1000000 * tv.tv_sec + tv.tv_usec;
}
