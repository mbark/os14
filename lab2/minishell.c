#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH (70)
#define MAX_NUM_ARGS (5)

void readCommand(char*, int);
void parseCommand(char*, char**, char**);
void cd(char*);
void quit();
void runCommandForeground(char*, char*[]);

int main(int argc, char* argv[]) {
  char buffer[MAX_COMMAND_LENGTH];
  char* command;
  char* args[MAX_NUM_ARGS + 1];
  
  while(1) {
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
      runCommandForeground(command, args);
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

void runCommandForeground(char* command, char* args[]) {
  pid_t pid = fork();
  if(pid == 0) {
    execvp(command, args);
    fprintf(stderr, "Unable to start command: %s\n", command);
    exit(1);
  } else if(pid > 0) {
    printf("Started foreground process with pid %d\n", pid);
    wait(NULL);
    printf("Foreground process %d ended\n", pid);
  } else {

  }
}
