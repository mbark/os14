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
void redirectStd(int, int);
void runProcess(int, int, char*, char**);

int pipe_fd1[2];
int pipe_fd2[2];
int pipe_fd3[2];

int main(int argc, char *argv[])
{
  pid_t pid;
  char* pager;
  char* args[1];

  createPipe(pipe_fd1);
  createPipe(pipe_fd2);
  createPipe(pipe_fd3);

  args[0] = "printenv";
  runProcess(0, pipe_fd1[1], "printenv", args);

  if(argc > 1) {
    argv[0] = "grep";
    runProcess(pipe_fd1[0], pipe_fd2[1], "grep", argv);
  } else {
    dup2(pipe_fd1[0], pipe_fd2[0]);
  }

  args[0] = "sort";
  runProcess(pipe_fd2[0], pipe_fd3[1], "sort", args);

  pid = fork();
  if(pid == 0) {
    pager = getenv("PAGER");
    redirectStd(pipe_fd3[0], 1);
    if(pager != NULL) {
      execlp(pager, pager, (char*) 0);
    }
    execlp("less", "less", (char*) 0);
    execlp("more", "more", (char*) 0);
    fprintf(stderr, "Unable to find a pager");
    exit(1);
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
  redirectStd(in, out);
  execvp(command, args);
}

void redirectStd(int in, int out) {
  dup2(in, 0);
  dup2(out, 1);

  closePipes();
}

void runProcess(int in, int out, char* command, char** args) {
  pid_t pid = fork();
  if(pid == 0) {
    runCommandArgs(in, out, command, args);
  }
}
