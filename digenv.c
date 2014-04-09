#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void createPipe(int*);
void closePipes();
void runCommand(int, int, char*);
void runCommandArgs(int, int, char*, char**);

int pipe_fd1[2];
int pipe_fd2[2];
int pipe_fd3[2];

int main(int argc, char *argv[])
{
  pid_t pid;

  createPipe(pipe_fd1);
  createPipe(pipe_fd2);
  createPipe(pipe_fd3);

  pid = fork();
  if(pid == 0) {
    runCommand(0, pipe_fd1[1], "printenv");
  }

  if(argc > 1) {
    pid = fork();
    if(pid == 0) {
      argv[0] = "grep";
      runCommandArgs(pipe_fd1[0], pipe_fd2[1], "grep", argv);
    }
  } else {
    dup2(pipe_fd1[0], pipe_fd2[0]);
  }

  pid = fork();
  if(pid == 0) {
    runCommand(pipe_fd2[0], pipe_fd3[1], "sort");
  }

  pid = fork();
  if(pid == 0) {
    runCommand(pipe_fd3[0], 1, "less");
  }

  closePipes();
  waitpid(pid, NULL, 0);
  
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
  close(pipe_fd3[0]);
  close(pipe_fd3[1]);
}

void runCommand(int in, int out, char* command) {
  char* args[1];
  args[0] = command;
  runCommandArgs(in, out, command, args);
}

void runCommandArgs(int in, int out, char* command, char** args) {
  dup2(in, 0);
  dup2(out, 1);
    
  closePipes();
  execvp(command, args);
}
