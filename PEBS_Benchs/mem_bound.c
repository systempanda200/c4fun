#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>
#include <numa.h>
#include <assert.h>
#include <sys/time.h>

// For perf
#include <linux/perf_event.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>

long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                int cpu, int group_fd, unsigned long flags) {
    int ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
                   group_fd, flags);
    return ret;
}

#define ELEM_TYPE uint64_t

/**
 * Structure used along with the following compare method to shuffle
 * an array of N elements.
 */
struct rand_struct {
  int index;
  int rand;
};

static int compar(const void* a1, const void* a2) {
  struct rand_struct *a = (struct rand_struct*) a1;
  struct rand_struct *b = (struct rand_struct*) a2;
  return a->rand - b->rand;
}

/**
 * Allocates and fills a memory region of the given size with values
 * indicating the adress of the next element in the memory region.
 */
uint64_t *alloc_fill_memory_sequential(size_t size) {

  uint64_t *memory = malloc(size);
  assert(memory);
  int nb_elems = size / sizeof(ELEM_TYPE);

  int i;
  for(i = 0; i < nb_elems - 1; i++) {
    memory[i] = (uint64_t)&memory[i + 1];
  }
  memory[i] = (uint64_t)&memory[0];

  return memory;
}

/**
 * Allocates and fills a memory region of the given size with values
 * indicating the adress an other random element in the memory
 * region. All the elements adresses are present in the filled region.
 */
uint64_t *alloc_fill_memory_rand(size_t size) {

  /**
   * Allocates the memory region taht will be returned.
   */
  uint64_t *memory = malloc(size);
  assert(memory);

  /**
   * Create another memory region and shuffle it.
   */
  size_t nb_elems = size / sizeof(ELEM_TYPE);
  size_t rand_memory_size = nb_elems * sizeof(struct rand_struct);
  struct rand_struct *rand_memory = malloc(rand_memory_size);
  assert(rand_memory);
  unsigned int seed = 1;
  for (int i = 0; i < nb_elems; i++) {
    rand_memory[i].index = i;
    rand_memory[i].rand = rand_r(&seed);
  }
  qsort(&rand_memory[1], nb_elems - 1, sizeof(*rand_memory), compar);

  /**
   * Fills the returned memory region with pointers to a next random
   * element using the shuffled memory region.
   */
  int i;
  for(i = 0; i < nb_elems - 1; i++) {
    memory[i] = (uint64_t)&memory[rand_memory[i + 1].index];
  }
  memory[i] = (uint64_t)&memory[0];

  return memory;
}

/**
 * Read the given memory region. Several accesses are done in each
 * loop iteration to limit the number of accesses caused by the loop
 * test.
 */
void read_memory(uint64_t *memory, size_t size) {

  uint64_t *addr = memory;
  int64_t nb_elem_remaining = size / sizeof(ELEM_TYPE);
  while (nb_elem_remaining > 0) {

    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;

    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;

    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;

    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;
    addr = (uint64_t *)*addr;

    nb_elem_remaining -= 64;
  }
}

enum access_mode_t {
  access_seq,
  access_random
};

void usage(const char *prog_name) {
  printf ("Usage %s size access_mode\n\tsize is the size of the allocated and accessed memory in bytes\n\taccess_mode is either seq for sequential accesses or rand for random accesses\n", prog_name);
}

int main(int argc, char **argv) {

  /**
   * Check and get arguments.
   */
  if (argc <= 2) {
    usage(argv[0]);
    return -1;
  }
  size_t size = atol(argv[1]);
  enum access_mode_t access_mode;
  if (!strcmp(argv[2], "seq")) {
    access_mode = access_seq;
  } else if (!strcmp(argv[2], "rand")) {
    access_mode = access_random;
  } else {
    printf("Unknown access_mode %s\n", argv[2]);
    usage(argv[0]);
    return -1;
  }
  printf("Running on %lu mega bytes\n", (size / 1000000));

  /**
   * Allocates and fills memory. Because the memory is filled, all its
   * pages are touched and as a consequence, no page faults will occur
   * during the measurement.
   */
  uint64_t *memory;
  if (access_mode == access_seq) {
    memory = alloc_fill_memory_sequential(size);
  } else {
    memory = alloc_fill_memory_rand(size);
  }

  /**
   * Profile memory reading
   */

  // Set attribute parameter for perf_event_open
  struct perf_event_attr pe_attr;
  memset(&pe_attr, 0, sizeof(pe_attr));
  pe_attr.size = sizeof(pe_attr);
  pe_attr.type = 6;
  pe_attr.config = 0x072C;
  pe_attr.type = PERF_TYPE_HARDWARE;
  pe_attr.config = PERF_COUNT_HW_INSTRUCTIONS;
  pe_attr.disabled = 1;
  pe_attr.exclude_kernel = 1;
  pe_attr.exclude_hv = 1;

  // Open the events with Linux system call
  printf("%d, %llu, %u\n", pe_attr.size, pe_attr.config,  pe_attr.type);
  int fd = perf_event_open(&pe_attr, -1, 2, -1, 0);
  if (fd == -1) {
    printf("perf_event_open_failed\n");
    return -1;
  }

  // Starts measuring
  struct timeval t1, t2;
  double elapsedTime;
  gettimeofday(&t1, NULL);
  ioctl(fd, PERF_EVENT_IOC_RESET, 0);
  ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);

  // Access memory
  read_memory(memory, size);

  // Stop measuring and print results
  int64_t count;
  ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
  read(fd, &count, sizeof(count));
  gettimeofday(&t2, NULL);
  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
  printf("time = %.3f ms\n", elapsedTime);
  printf("memory reads = %" PRId64 "\n", count);

  close(fd);
  free(memory);
  return 0;
}
