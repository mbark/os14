#include <stdio.h>
#include <stdlib.h>
#include "../brk.h"
#include <unistd.h>
#include <malloc.h>

#define SIZE (1)

int main(int argc, char *argv[]) {
  long num;
  long i;
  int sum = 0;

  num = atol(argv[1]);

  for(i = 0; i < num; i++) {
    malloc(SIZE);
    sum += SIZE;
  }

  fprintf(stderr, "Sum: %d\n", sum);
  malloc_stats();

  return 0;
}
