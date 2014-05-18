#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

long _start_time;
long _elapsed_time;

long get_time() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return 1000000 * tv.tv_sec + tv.tv_usec;
}

void _reset_clock() {
  _elapsed_time = 0;
}

void _pause() {
  _elapsed_time += (get_time() - _start_time);
}

void _resume() {
  _start_time = get_time();
}

long _get_elapsed_time() {
  return _elapsed_time;
}

void _print_elapsed_time() {
  double sec = _get_elapsed_time()/1000000.0;
  fprintf(stderr, "Elapsed time sec: %.5f\n", sec);
}
