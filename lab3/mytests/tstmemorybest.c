#include <stdio.h>
#include <stdlib.h>
#include "../brk.h"
#include <unistd.h>

#define SIZE (16368)

int main(int argc, char *argv[]) {
  long num;
  long i;
  char *lowbreak;
  char *highbreak;
  int sum = 0;

  num = atol(argv[1]);

  lowbreak = (char*) endHeap();
  
  for(i = 0; i < num; i++) {
    malloc(SIZE);
    sum += SIZE;
  }

  highbreak = (char*) endHeap();

  fprintf(stderr, "Low break: %p\nHigh break: %p\n", lowbreak, highbreak);
  fprintf(stderr, "Break difference: %lu\n", (highbreak - lowbreak));
  fprintf(stderr, "Allocated: %d\n", sum);
  fprintf(stderr, "Overhead: %lu\n", (highbreak - lowbreak) - sum);

  return 0;
}
