#include <stdio.h>
#include <stdlib.h>
#include "../brk.h"
#include <unistd.h>
#include "clock.h"

int main(int argc, char *argv[]) {
  int nbytes = 5;
  void **ptrs;
  long num;
  long i;

  num = atol(argv[1]);

  ptrs = malloc(sizeof(void*) * num);

  for(i = 0; i < num; i++) {
    ptrs[i] = malloc(nbytes);
    malloc(1);
  }

  for(i = 0; i < num; i++) {
    free(ptrs[i]);
  }

  _reset_clock();
  for(i = 0; i < num; i++) {
    _resume();
    ptrs[i] = malloc(nbytes + 1);
    _pause();
  }

  _print_elapsed_time();

  return 0;
}
