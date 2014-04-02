#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void createPipe(int*);

int main()
{
  int five = 55;
  int pipe_fd1[2];
  int pipe_fd2[2];
  int pipe_fd3[2];
  pid_t pid;

  createPipe(pipe_fd1);
  createPipe(pipe_fd2);
  /* createPipe(pipe_fd3); */

  pid = fork();
  if(pid == 0) {
    dup2(pipe_fd1[0], 0);
    dup2(pipe_fd2[1], 1);
    close(pipe_fd1[0]);
    close(pipe_fd1[1]);
    close(pipe_fd2[0]);
    close(pipe_fd2[1]);
    execlp("cat", "cat", (char*) 0);
    /*int number = 0;
    read(STDIN_FILENO, &number, sizeof(number));
    number += 3;
    write(STDOUT_FILENO, &number, sizeof(number));
    exit(0); */
  }

  pid = fork();
  if(pid == 0) {
    dup2(pipe_fd2[0], 0);
    close(pipe_fd1[0]);
    close(pipe_fd1[1]);
    close(pipe_fd2[0]);
    close(pipe_fd2[1]);
    /* dup2(pipe_fd3[1], 1); */
    execlp("cat", "cat", (char*) 0);
    /*int number = 0;
    read(STDIN_FILENO, &number, sizeof(number));
    number += 7;
    write(STDOUT_FILENO, &number, sizeof(number));
    exit(0);*/
  }

  /* int number = 0; */
  write(pipe_fd1[1], &five, sizeof(five));
  close(pipe_fd1[0]);
  close(pipe_fd1[1]);
  close(pipe_fd2[0]);
  close(pipe_fd2[1]);
  /* read(pipe_fd3[0], &number, sizeof(number));
     fprintf(stderr, "winning: %d\n", number); */

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
