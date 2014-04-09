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
pid_t runProcess(int, int*, char*, char**);
void waitForChild();

int pipe_fd1[2];
int pipe_fd2[2];
int pipe_fd3[2];

int main(int argc, char *argv[])
{
  pid_t pid;
  char* pager;
  char* args[1];

  createPipe(pipe_fd1);

  args[0] = "printenv";
  runProcess(0, pipe_fd1, "printenv", args);
  waitForChild();
  close(pipe_fd1[1]);

  createPipe(pipe_fd2);

  if(argc > 1) {
    argv[0] = "grep";
    runProcess(pipe_fd1[0], pipe_fd2, "grep", argv);
    waitForChild();
    close(pipe_fd2[1]);
  } else {
    dup2(pipe_fd1[0], pipe_fd2[0]);
  }

  createPipe(pipe_fd3);
  
  args[0] = "sort";
  runProcess(pipe_fd2[0], pipe_fd3, "sort", args);
  waitForChild();
  close(pipe_fd3[1]);

  pid = fork();
  if(pid == 0) {
    pager = getenv("PAGER");
    dup2(pipe_fd3[0], 0);
    if(pager != NULL) {
      execlp(pager, pager, (char*) 0);
    }
    execlp("less", "less", (char*) 0);
    execlp("more", "more", (char*) 0);
    fprintf(stderr, "Unable to find a pager");
    exit(1);
  }

  waitForChild();
  close(pipe_fd1[0]);
  close(pipe_fd2[0]);
  close(pipe_fd3[0]);
  
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

pid_t runProcess(int in, int* pipe_out, char* command, char** args) {
  pid_t pid = fork();
  if(pid == 0) {
    dup2(in, 0);
    dup2(pipe_out[1], 1);
    close(in);
    close(pipe_out[0]);
    close(pipe_out[1]);
    execvp(command, args);
  }

  return pid;
}

void waitForChild() {
  int status;
  wait(&status);
  if(!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
    fprintf(stderr, "Child terminated abnormally\n");
    exit(1);
  }
}
