#ifndef PEBS_BENCH_H
#define PEBS_BENCH_H

#include <linux/perf_event.h>
#include <inttypes.h>
#include <unistd.h>

#include <sys/syscall.h>

#define rmb() asm volatile("lfence" ::: "memory")

/**
 * Structure representing a sample gathered with the library in sampling mode.
 */
struct sample {
  uint64_t ip;
  uint64_t addr;
  uint64_t weight;
  union perf_mem_data_src data_src;
};

#endif
