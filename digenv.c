#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void createPipe(int*);
void closePipes();
void runCommand(int, int, char*);

int pipe_fd1[2];
int pipe_fd2[2];

int main()
{
  pid_t pid;

  createPipe(pipe_fd1);
  createPipe(pipe_fd2);

  pid = fork();
  if(pid == 0) {
    runCommand(0, pipe_fd1[1], "printenv");
  }

  pid = fork();
  if(pid == 0) {
    runCommand(pipe_fd1[0], pipe_fd2[1], "sort");
  }

  runCommand(pipe_fd2[0], 1, "less");
  exit(0);
}

void createPipe(int* pipe_fd) {
  int return_value;
  return_value = pipe(pipe_fd);
  if(return_value == -1) {
    fprintf(stderr, "Unable to create pipe");
    exit(1);
  }
}

void closePipes() {
  close(pipe_fd1[0]);
  close(pipe_fd1[1]);
  close(pipe_fd2[0]);
  close(pipe_fd2[1]);
}


void runCommand(int in, int out, char* command) {
  dup2(in, 0);
  dup2(out, 1);
    
  closePipes();
  execlp(command, command, (char*) 0);
}
