#include <stdio.h>
#include <stdlib.h>
#include "../brk.h"
#include <unistd.h>
#include "clock.h"

void run_test(int NUM) {
  int i;
  void *ptr;

  _reset_clock();
  for(i = 0; i < NUM; i++) {
    _resume();
    ptr = malloc(100);
    _pause();
    free(ptr);
  }

  _print_elapsed_time();
}

int main(int argc, char *argv[]) {
  int i;

  for(i = 100; i < 1000000000; i *= 10) {
    run_test(i);
  }

  return 0;
}
