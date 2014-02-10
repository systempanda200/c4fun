#include "mem_alloc.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>
#include <assert.h>
#include <time.h> // For clock_gettime
#include <sys/time.h> // For gettimeofday
#include <string.h>
#include <cpuid.h>

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


unsigned int l1, l2, l3;

void i386_cpuid_caches () {
    int i;
    for (i = 0; i < 32; i++) {

        // Variables to hold the contents of the 4 i386 legacy registers
        uint32_t eax, ebx, ecx, edx;

        eax = 4; // get cache info
        ecx = i; // cache id

        __asm__ (
            "cpuid" // call i386 cpuid instruction
            : "+a" (eax) // contains the cpuid command code, 4 for cache query
            , "=b" (ebx)
            , "+c" (ecx) // contains the cache id
            , "=d" (edx)
        ); // generates output in 4 registers eax, ebx, ecx and edx

        // taken from http://download.intel.com/products/processor/manual/325462.pdf Vol. 2A 3-149
        int cache_type = eax & 0x1F;

        if (cache_type == 0) // end of valid cache identifiers
            break;

        char * cache_type_string;
        switch (cache_type) {
            case 1: cache_type_string = "Data Cache"; break;
            case 2: cache_type_string = "Instruction Cache"; break;
            case 3: cache_type_string = "Unified Cache"; break;
            default: cache_type_string = "Unknown Type Cache"; break;
        }

        int cache_level = (eax >>= 5) & 0x7;

        int cache_is_self_initializing = (eax >>= 3) & 0x1; // does not need SW initialization
        int cache_is_fully_associative = (eax >>= 1) & 0x1;


        // taken from http://download.intel.com/products/processor/manual/325462.pdf 3-166 Vol. 2A
        // ebx contains 3 integers of 10, 10 and 12 bits respectively
        unsigned int cache_sets = ecx + 1;
        unsigned int cache_coherency_line_size = (ebx & 0xFFF) + 1;
        unsigned int cache_physical_line_partitions = ((ebx >>= 12) & 0x3FF) + 1;
        unsigned int cache_ways_of_associativity = ((ebx >>= 10) & 0x3FF) + 1;

        // Total cache size is the product
        size_t cache_total_size = cache_ways_of_associativity * cache_physical_line_partitions * cache_coherency_line_size * cache_sets;

        printf(
            "Cache ID %d:\n"
            "- Level: %d\n"
            "- Type: %s\n"
            "- Sets: %d\n"
            "- System Coherency Line Size: %d bytes\n"
            "- Physical Line partitions: %d\n"
            "- Ways of associativity: %d\n"
            "- Total Size: %zu bytes (%zu kb)\n"
            "- Is fully associative: %s\n"
            "- Is Self Initializing: %s\n"
            "\n"
            , i
            , cache_level
            , cache_type_string
            , cache_sets
            , cache_coherency_line_size
            , cache_physical_line_partitions
            , cache_ways_of_associativity
            , cache_total_size, cache_total_size >> 10
            , cache_is_fully_associative ? "true" : "false"
            , cache_is_self_initializing ? "true" : "false"
        );
	if (cache_level == 1 && cache_type == 1) {
	  l1 = cache_total_size;
	} else if (cache_level == 2) {
	  l2 = cache_total_size;
	} else if (cache_level == 3) {
	  l3 = cache_total_size;
	}
    }
}

void usage(const char *prog_name) {
  printf ("Usage %s: max_size_KiB nb_reads mode (where mode is either seq or rand)\n", prog_name);
}

int main(int argc, char **argv) {

 /**
  * Check and get arguments.
  */
  if (argc != 4) {
    usage(argv[0]);
    return -1;
  }
  size_t max_size = atol(argv[1]) * 1024;
  size_t nb_reads = atol(argv[2]);
  enum access_mode_t access_mode;
  if (!strcmp(argv[3], "seq")) {
    access_mode = access_seq;
  } else if (!strcmp(argv[3], "rand")) {
    access_mode = access_rand;
  } else {
    printf("Unknown access_mode %s\n", argv[3]);
    usage(argv[0]);
    return -1;
  }

  i386_cpuid_caches();
  printf("L1 = %u\n", l1);
  printf("L2 = %u\n", l2);
  printf("L3 = %u\n\n", l3);

  /**
   * Measure time to measure time :-)
   */
  struct timespec start, end;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
  int nb_rep = 1E6;
  for (int i = 0; i < nb_rep; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
  }
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
  uint64_t ellapsed = (end.tv_sec * 1E9 + end.tv_nsec) - (start.tv_sec * 1E9 + start.tv_nsec);
  printf("Time for clock_gettime = %" PRIu64 " nanoseconds (ellapsed = %ld)\n", ellapsed / nb_rep, ellapsed);

  /**
   * Measure time to measure time with an other function :-)
   */
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
  struct timeval tv;
  for (int i = 0; i < nb_rep; i++) {
    gettimeofday(&tv, NULL);
  }
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
  ellapsed = (end.tv_sec * 1E9 + end.tv_nsec) - (start.tv_sec * 1E9 + start.tv_nsec);
  printf("Time for gettimeofday  = %" PRIu64 " nanoseconds (ellapsed = %ld)\n\n", ellapsed / nb_rep, ellapsed);

  printf("%-10s %-10s", "Size (KiB)", "Time (ns)\n");
  for (size_t size = 1024; size <= max_size; size = step(size)) {
    uint64_t *memory = malloc(size);
    assert(memory);
    fill_memory(memory, size, access_mode);
    register long remaining = nb_reads;
    register uint64_t *p = memory;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    while (remaining > 0) {
      HUNDRED
      remaining -= 100;
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
    ellapsed = (end.tv_sec * 1E9 + end.tv_nsec) - (start.tv_sec * 1E9 + start.tv_nsec);
    printf("%-10zu %-10f" "\n", size / 1024, ellapsed / (float)nb_reads);
  }
  return 0;
}
