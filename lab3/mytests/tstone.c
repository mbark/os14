#include <stdio.h>
#include <stdlib.h>
#include "../brk.h"
#include <unistd.h>
#include "clock.h"

#define NUM 10000000

int main(int argc, char *argv[]) {
  int i;
  void *ptr;

  _reset_clock();
  for(i = 0; i < NUM; i++) {
    _resume();
    ptr = malloc(1);
    _pause();
  }

  fprintf(stderr, "Total malloc time: %lu\n", _get_elapsed_time());

  return 0;
}
