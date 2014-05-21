#include <stdio.h>
#include <stdlib.h>
#include "../brk.h"
#include <unistd.h>
#include "clock.h"

int main(int argc, char *argv[]) {
  long num;
  long i;
  void *ptr;

  num = atol(argv[1]);

  _reset_clock();
  for(i = 0; i < num; i++) {
    _resume();
    ptr = malloc(100);
    _pause();
    free(ptr);
  }

  _print_elapsed_time();

  return 0;
}
