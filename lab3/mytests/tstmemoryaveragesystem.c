#include <stdio.h>
#include <stdlib.h>
#include "../brk.h"
#include <unistd.h>
#include <time.h>
#include <malloc.h>

#define MIN_ALLOC (1)
#define MAX_ALLOC (16368)

int get_random_size() {
  return MIN_ALLOC + (rand() % (MAX_ALLOC - MIN_ALLOC));
}

int main(int argc, char *argv[]) {
  long num;
  long i;
  int size;
  int sum = 0;
  long overhead;
  struct mallinfo info;

  srand((unsigned int)time(NULL));

  num = atol(argv[1]);

  for(i = 0; i < num; i++) {
    size = get_random_size();
    malloc(size);
    sum += size;
  }

  fprintf(stderr, "Sum: %d\n", sum);
  malloc_stats();

  info = mallinfo();
  fprintf(stderr, "info.arena: %d\n", info.arena);

  overhead = info.arena - sum;
  fprintf(stderr, "%lu (%lu%%)\n", overhead, 100*overhead / info.arena);

  return 0;
}
