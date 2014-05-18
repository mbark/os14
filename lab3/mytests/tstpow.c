#include <stdio.h>
#include <stdlib.h>
#include "../brk.h"
#include <unistd.h>
#include "clock.h"

#define NUM 100000000

void *ptrs[NUM];
void *stup[NUM];

void run_test(int runs) {
  int i;
  int nbytes = 200;

  _reset_clock();
  _resume();
  for(i = 0; i < runs; i++) {
    ptrs[i] = malloc(nbytes);
    malloc(1);
  }
  _pause();
  _print_elapsed_time();

  _reset_clock();
  _resume();
  for(i = 0; i < runs; i++) {
    free(ptrs[i]);
  }
  _pause();
  _print_elapsed_time();

  _reset_clock();
  _resume();
  for(i = 0; i < runs; i++) {
    ptrs[i] = malloc(nbytes + 1);
  }
  _pause();

  _print_elapsed_time();
}

int main(int argc, char *argv[]) {
  int i;
  for(i = 100; i < NUM; i *= 10) {
    fprintf(stderr, "i = %d\n", i);
    run_test(i);
  }

  return 0;
}
