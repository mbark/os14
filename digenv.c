/*
digenv is a program that allows for easier retrieval and filtering of the environmental
variables. You can run it with or without parameters. When digenv is run with no
parameters it is equivalent to running printenv | sort | THEPAGER. Where THEPAGER is
the pager specified in the PAGER environmental variable. If the PAGER environmental
variable is unspecified digenv will attempt to use less as the pager, if this fails
more will be used. If digenv is run with parameters as digenv [parameters] it is
equivalent to running printenv | grep [parameters] | sort | THEPAGER.
 */
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
void runProcess(int, int*, char*, char**);
void waitForChild();
void dup2Safe(int, int);
void closeSafe(int);

/* Pipe used to communicate between printenv and grep. */
int pipe_fd1[2];
/* Pipe used to communicate between grep and sort. */
int pipe_fd2[2];
/* Pipe used to communicate between sort and THEPAGER. */
int pipe_fd3[2];

/* Starting point of the program. This function will set up the pipes and start the
child processes for the printenv, grep, sort and THEPAGER commands. It will ensure
that the pipes are correctly connected between the commands. Main starts one child
at a time and waits until the child has terminated before starting the next child
in the chain.
argc is the number of command line arguments to digenv.
argv is an array holding the command line arguments of digenv. The first value
in the array should always be the name of this command.
*/
int main(int argc, char *argv[])
{
  /* Process id of the process of THEPAGER. */
  pid_t pid;
  /* Used to hold the value of the PAGER environmental variable. */
  char* pager;
  /* Used to hold arguments for the printenv and sort commands. */
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
    /* Pipe printenv directly into sort skipping the grep step. */
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
    /* If the PAGER environmental variable is set attempts to use
       that as the pager. Otherwise fall back to less and then more. */
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

/* Creates a pipe. If an error occurs error output is printed to
   stderr and the program is terminated.
   pipe_fd is the array where the file descriptors of the read and
   write ends of the pipe will be stored. The read end will be stored
   in pipe_fd[0] and the write end will be stored in read_fd[1]. */
void createPipe(int* pipe_fd) {
  /* Holds the return value of the pipe system call. */
  int return_value = pipe(pipe_fd);
  if(return_value == -1) {
    fprintf(stderr, "Unable to create pipe");
    exit(1);
  }
}

/* This function starts a new process by calling fork. The child process
   redirects stdin stdout to the in and out pipes of the function. It then
   executes the given command with the given arguments by running exec.
   in is the file descriptor that will be used as stdin.
   pipe_out[1] is the file descriptor that will be used as stdout.
   command is the name of the command that will be executed.
   args is the arguments that will be passed to the command. The first argument
   should always be the name of the command.
*/
void runProcess(int in, int* pipe_out, char* command, char** args) {
  /* Holds the return value of the fork call. Used to determine if the process
     is the child or the parent. */
  pid_t pid = fork();
  if(pid == 0) {
    dup2Safe(in, 0);
    dup2Safe(pipe_out[1], 1);
    /* Close all open pipes to ensure that the children terminates correctly. */
    closeSafe(in);
    closeSafe(pipe_out[0]);
    closeSafe(pipe_out[1]);
    execvp(command, args);
    fprintf(stderr, "Unable to execute command %s", command);
    exit(1);
  }
}

/* This function waits until any child terminates at which point it checks
   if the child terminated successfully. If the child did not, it prints
   an error message to stderr and exits the process. */
void waitForChild() {
  /* Holds the exit status of the child that terminated. */
  int status;
  /* Holds the pid of the child that terminated. */
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

/* dup2Safe is the same as the dup2 function but if an error occurs it
   prints an error message and exits the process.
   oldfd is equivalent to the oldfd parameter of dup2.
   newfd is equivalent to the newfd parameter of dup2. */
void dup2Safe(int oldfd, int newfd) {
  /* Holds the return value of the dup2 call. */
  int return_value = dup2(oldfd, newfd);
  if(return_value == -1) {
    fprintf(stderr, "Unable to execute dup2(%d, %d)", oldfd, newfd);
    exit(1);
  }
}

/* closeSafe is equivalent to the close function but if an error occurs it
   prints an error message and exits the process.
   fd is equivalent to the fd parameter of close. */
void closeSafe(int fd) {
  /* Holds the return value of the close call. */
  int return_value = close(fd);
  if(return_value == -1) {
    fprintf(stderr, "Unable to execute close(%d)", fd);
    exit(1);
  }
}
