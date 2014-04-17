#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_COMMAND_LENGTH (70)
#define MAX_NUM_ARGS (5)

void readCommand(char*, int);
void parseCommand(char*, char**, char**);
void cd(char*);
void getDirectory(char*[]);
void quit();
void runCommand(char*);

int main(int argc, char* argv[]) {
  char buffer[MAX_COMMAND_LENGTH];
  char* command;
  char* args[MAX_NUM_ARGS];
  
  printf("minishell>");
  readCommand(buffer, MAX_COMMAND_LENGTH);
  parseCommand(buffer, &command, args);
  if(strcmp(command, "exit") == 0) {
    quit();
  } else if(strcmp(command, "cd") == 0) {
    cd("");
  } else {
    runCommand(command);
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
  for(i = 0; i < MAX_NUM_ARGS; i++) {
    args[i] = strtok(NULL, " ");
  }
}

void cd(char* path) {
  printf("cd\n");
}

void getDirectory(char* args[]) {

}

void quit() {
  printf("quit\n");
  exit(0);
}

void runCommand(char* command) {
  /*execlp(buffer, buffer, (char*) 0);
  fprintf(stderr, "Unable to start command: %s\n", buffer);
  exit(1);*/
}
