#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void runCommand(int*, int, char*);

int main()
{
  int fd[2];
  int return_value;
  return_value = pipe(fd);
  if(return_value == -1) {
    fprintf(stderr, "Unable to create pipe");
    exit(1);
  }

  runCommand(fd, 0, "sort");
  runCommand(fd, 1, "printenv");
  exit(0);
}

void runCommand(int* fd, int index, char* command) {
  pid_t pid;
  pid = fork();
  if(pid == 0) {
    dup2(fd[index], index);
    close(fd[0]);
    close(fd[1]);
    execlp(command, command, (char*) 0);
    /* If execlp succeeds it does not return, so if we get it failed. */
    fprintf(stderr, "Unable to start command %s", command);
    exit(1);
  } else if(pid == -1) {
    fprintf(stderr, "Unable to fork");
    exit(1);
  }
}
