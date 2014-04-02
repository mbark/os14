#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void createPipe(int*);

int main()
{
  int pipe_fd1[2];
  int pipe_fd2[2];
  pid_t pid;

  createPipe(pipe_fd1);
  createPipe(pipe_fd2);

  pid = fork();
  if(pid == 0) {
    dup2(pipe_fd1[1], 1);
    
    close(pipe_fd1[0]);
    close(pipe_fd1[1]);
    close(pipe_fd2[0]);
    close(pipe_fd2[1]);
    execlp("printenv", "printenv", (char*) 0);
  }

  pid = fork();
  if(pid == 0) {
    dup2(pipe_fd1[0], 0);
    dup2(pipe_fd2[1], 1);
    
    close(pipe_fd1[0]);
    close(pipe_fd1[1]);
    close(pipe_fd2[0]);
    close(pipe_fd2[1]);
    execlp("sort", "sort", (char*) 0);
  }

  dup2(pipe_fd2[0], 0);
  close(pipe_fd1[0]);
  close(pipe_fd1[1]);
  close(pipe_fd2[0]);
  close(pipe_fd2[1]);
  execlp("less", "less", (char*) 0);

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
