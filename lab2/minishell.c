#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void readCommand(char*, int);

int main(int argc, char* argv[]) {
  char buffer[70];
  
  printf("minishell>");
  readCommand(buffer, 70);
  
  execlp(buffer, buffer, (char*) 0);
  fprintf(stderr, "Unable to start command: %s\n", buffer);
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
