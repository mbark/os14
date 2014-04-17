#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_COMMAND_LENGTH (70)
#define MAX_NUM_ARGS (5)

void readCommand(char*, int);
void parseCommand(char*, char*, char*[MAX_NUM_ARGS]);
void cd(char*);
void getDirectory(char*, char*);
void quit();
void runCommand(char*);

int main(int argc, char* argv[]) {
  char buffer[MAX_COMMAND_LENGTH];
  char command[MAX_COMMAND_LENGTH];
  char* args[MAX_NUM_ARGS];
  
  printf("minishell>");
  readCommand(buffer, MAX_COMMAND_LENGTH);
  parseCommand(buffer, command, args);
  if(strcmp(buffer, "exit") == 0) {
    quit();
  } else if(strcmp(buffer, "cd") == 0) {
    getDirectory(buffer, buffer);
    /* cd(); */
  } else {
    runCommand(buffer);
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

void parseCommand(char* buffer, char* command, char* args[MAX_NUM_ARGS]) {
  int i;
  command = strtok(buffer, " ");
  printf("Command: %s", command);
  for(i = 0; i < MAX_NUM_ARGS; i++) {
    args[i] = strtok(NULL, " ");
    printf("Arg%d: %s", i, args[i]);
  }
}

void cd(char* path) {
  
}

void getDirectory(char* buffer, char* path) {
  char* string = strtok(buffer, " ");
  printf("got: %s", string);
}

void quit() {
  exit(0);
}

void runCommand(char* command) {
  /*execlp(buffer, buffer, (char*) 0);
  fprintf(stderr, "Unable to start command: %s\n", buffer);
  exit(1);*/
}
