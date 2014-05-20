#include <stdio.h>
#include <stdlib.h>
#include "../brk.h"
#include <unistd.h>
#include <time.h>

#define MIN_ALLOC (1)
#define MAX_ALLOC (16368)

int get_random_size() {
  return MIN_ALLOC + (rand() % (MAX_ALLOC - MIN_ALLOC));
}

int main(int argc, char *argv[]) {
  long num;
  long i;
  char *lowbreak;
  char *highbreak;
  int size;
  int sum = 0;
  long consumed;
  long overhead;

  srand((unsigned int)time(NULL));

  num = atol(argv[1]);

  lowbreak = (char*) endHeap();
  
  for(i = 0; i < num; i++) {
    size = get_random_size();
    malloc(size);
    sum += size;
  }

  highbreak = (char*) endHeap();

  consumed = highbreak - lowbreak;
  overhead = consumed - sum;
  fprintf(stderr, "Low break: %p\nHigh break: %p\n", lowbreak, highbreak);
  fprintf(stderr, "Break difference: %lu\n", (highbreak - lowbreak));
  fprintf(stderr, "Allocated: %d\n", sum);
  fprintf(stderr, "Overhead: %lu (%lu%%)\n", overhead, 100 * overhead / consumed);

  return 0;
}
