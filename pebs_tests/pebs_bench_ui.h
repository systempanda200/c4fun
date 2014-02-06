#ifndef PEBS_BENCH_UI_H
#define PEBS_BENCH_UI_H

#include "pebs_bench.h"

typedef enum {
  IP,
  ADDR,
  IN_OUT_MEMORY,
  WEIGHT,
  SRC
}
display_order;

void print_samples(struct perf_event_mmap_page *metadata_page, display_order order, uint64_t start_addr, uint64_t end_addr, int nb_samples_estimated);
char* concat(const char *s1, const char *s2);

#endif
