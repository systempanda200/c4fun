#include "mem_alloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>
#include <assert.h>
#include <time.h>

#define MAX  30000 * 1204

#define ONE     p = (uint64_t *)*p;
#define FIVE    ONE ONE ONE ONE ONE
#define	TEN	FIVE FIVE
#define	FIFTY	TEN TEN TEN TEN TEN
#define	HUNDRED	FIFTY FIFTY

size_t step(size_t k) {
  if (k < 1024) {
    k = k * 2;
  } else if (k < 4*1024) {
    k += 1024;
  } else {
    size_t s;
    for (s = 4 * 1024; s <= k; s *= 2) {;}
    k += s / 4;
  }
  return k;
}

int main() {

  struct timespec start, end;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
  int nb_rep = 1000000;
  for (int i = 0; i < nb_rep; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
  }
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
  uint64_t ellapsed = (end.tv_sec * 1000000000 + end.tv_nsec) - (start.tv_sec * 1000000000 + start.tv_nsec);
  printf("Time for clock_gettime = %" PRIu64 " microseconds (ellapsed = %ld)\n", ellapsed / nb_rep, ellapsed / 1000);

  printf("%-10s %-10s", "Size (KiB)", "Time (ms)\n");
  for (size_t size = 1024; size <= MAX; size = step(size)) {
    uint64_t *memory = malloc(size);
    assert(memory);
    fill_memory_seq(memory, size);
    register long remaining = 10000000;
    register uint64_t *p = memory;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    while (remaining > 0) {
      HUNDRED
	remaining -= 100;
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
    ellapsed = (end.tv_sec * 1000000000 + end.tv_nsec) - (start.tv_sec * 1000000000 + start.tv_nsec);
    printf("%-10zu %-10" PRIu64 "\n", size / 1024, ellapsed / 1000);
  }
  return 0;
}
