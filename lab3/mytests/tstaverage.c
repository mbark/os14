#include <stdio.h>
#include <stdlib.h>
#include "../brk.h"
#include <unistd.h>
#include <time.h>
#include "clock.h"

#define MIN_ALLOC (10)
#define MAX_ALLOC (100)
#define NUM_LOOPS (10)

int get_random_size() {
  return MIN_ALLOC + (rand() % (MAX_ALLOC - MIN_ALLOC));
}

int main(int argc, char *argv[]) {
  void **ptrs;
  long num;
  long i;
  int loop;

  srand((unsigned int)time(NULL));

  num = atol(argv[1]);
  ptrs = malloc(sizeof(void*) * num);

  for(i = 0; i < num; i++) {
    ptrs[i] = malloc(get_random_size());
  }

  _reset_clock();
  for(loop = 0; loop < NUM_LOOPS; loop++) {
    for(i = 0; i < num; i++) {
      if((rand() % 2) == 0) {
	free(ptrs[i]);
	ptrs[i] = NULL;
      }
    }

    for(i = 0; i < num; i++) {
      if(ptrs[i] == NULL) {
	_resume();
	ptrs[i] = malloc(get_random_size());
	_pause();
      }
    }
  }

  _print_elapsed_time();

  return 0;
}
