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
void dup2Safe(int, int);
void closeSafe(int);

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
  closeSafe(pipe_fd1[1]);

  createPipe(pipe_fd2);

  if(argc > 1) {
    argv[0] = "grep";
    runProcess(pipe_fd1[0], pipe_fd2, "grep", argv);
    waitForChild();
    closeSafe(pipe_fd2[1]);
  } else {
    dup2Safe(pipe_fd1[0], pipe_fd2[0]);
  }

  createPipe(pipe_fd3);
  
  args[0] = "sort";
  runProcess(pipe_fd2[0], pipe_fd3, "sort", args);
  waitForChild();
  closeSafe(pipe_fd3[1]);

  pid = fork();
  if(pid == 0) {
    pager = getenv("PAGER");
    dup2Safe(pipe_fd3[0], 0);
    if(pager != NULL) {
      execlp(pager, pager, (char*) 0);
    }
    execlp("less", "less", (char*) 0);
    execlp("more", "more", (char*) 0);
    fprintf(stderr, "Unable to find a pager");
    exit(1);
  }

  waitForChild();
  closeSafe(pipe_fd1[0]);
  closeSafe(pipe_fd2[0]);
  closeSafe(pipe_fd3[0]);
  
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
    dup2Safe(in, 0);
    dup2Safe(pipe_out[1], 1);
    closeSafe(in);
    closeSafe(pipe_out[0]);
    closeSafe(pipe_out[1]);
    execvp(command, args);
    fprintf(stderr, "Unable to execute command %s", command);
    exit(1);
  }

  return pid;
}

void waitForChild() {
  int status;
  pid_t pid = wait(&status);
  if(pid == -1) {
    fprintf(stderr, "Unable to wait for child");
    exit(1);
  }
  
  if(!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
    fprintf(stderr, "Child terminated abnormally\n");
    exit(1);
  }
}

void dup2Safe(int oldfd, int newfd) {
  int return_value = dup2(oldfd, newfd);
  if(return_value == -1) {
    fprintf(stderr, "Unable to execute dup2(%d, %d)", oldfd, newfd);
    exit(1);
  }
}

void closeSafe(int fd) {
  int return_value = close(fd);
  if(return_value == -1) {
    fprintf(stderr, "Unable to execute close(%d)", fd);
    exit(1);
  }
}
