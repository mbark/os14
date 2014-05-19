#include <stdio.h>
#include <stdlib.h>
#include "../brk.h"
#include <unistd.h>
#include "clock.h"

#define MIN_ALLOC (1024)
#define HEADER_SIZE (16)

int main(int argc, char *argv[]) {
  void **ptrs;
  long num;
  long i;

  int small_malloc = HEADER_SIZE;
  int medium_malloc = (MIN_ALLOC - 3) * HEADER_SIZE;
  int large_malloc = (MIN_ALLOC - 1) * HEADER_SIZE;

  num = atol(argv[1]);

  ptrs = malloc(sizeof(void*) * num);

  for(i = 0; i < num; i++) {
    ptrs[i] = malloc(medium_malloc);
    malloc(small_malloc);
  }

  for(i = 0; i < num; i++) {
    free(ptrs[i]);
  }

  _reset_clock();
  for(i = 0; i < num; i++) {
    _resume();
    ptrs[i] = malloc(large_malloc);
    _pause();
  }

  _print_elapsed_time();

  return 0;
}
