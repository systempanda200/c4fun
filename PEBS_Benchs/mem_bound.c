#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>
#include <numa.h>
#include <assert.h>
#include <sys/time.h>
#include <errno.h>
#include <err.h>
#include <sys/mman.h>

#include <perfmon/pfmlib_perf_event.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>

#define ELEM_TYPE uint64_t

#define rmb() asm volatile("lfence" ::: "memory")

int is_served_by_memory(union perf_mem_data_src data_src) {         
  if (data_src.mem_lvl & PERF_MEM_LVL_MISS) {                                                                                                                                  
    if (data_src.mem_lvl & PERF_MEM_LVL_L3) {                    
      return 1;
    } else {
      return 0;
    }
  } else if (data_src.mem_lvl & PERF_MEM_LVL_HIT) {
    if (data_src.mem_lvl & PERF_MEM_LVL_LOC_RAM) {
      return 1; 
    } else if (data_src.mem_lvl & PERF_MEM_LVL_REM_RAM1) {
      return 1; 
    } else if (data_src.mem_lvl & PERF_MEM_LVL_REM_RAM2) {
      return 1; 
    } else {
      return 0;
    }
  } else {
    return 0;
  }
}

/**
 * Structure representing a sample gathered with the library in sampling mode.
 */
struct sample {                                                                                                                                                                      uint64_t ip;
  uint64_t addr;
  uint64_t weight;
  union perf_mem_data_src data_src;
};


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

  // Manualy set and open memory counting event
  struct perf_event_attr pe_attr;
  memset(&pe_attr, 0, sizeof(pe_attr));
  pe_attr.size = sizeof(pe_attr);
  pe_attr.type = 6; // /sys/bus/event_source/uncore/type
  pe_attr.config = 0x072c; // QMC_NORMAL_READS.ANY
  pe_attr.disabled = 1;
  int memory_reads_fd = perf_event_open(&pe_attr, -1, 9, -1, 0);
  if (memory_reads_fd == -1) {
    printf("perf_event_open_failed: %s\n", strerror(errno));
    return -1;
  }

  // Manualy set and open memory sampling event
  struct perf_event_attr pe_attr_sampling;
  memset(&pe_attr_sampling, 0, sizeof(pe_attr_sampling));
  pe_attr_sampling.size = sizeof(pe_attr_sampling);
  pe_attr_sampling.type = PERF_TYPE_RAW;
  pe_attr_sampling.config = 0x0100b; //
  pe_attr_sampling.disabled = 1;
  pe_attr_sampling.config1 = 3; // latency threshold
  pe_attr_sampling.sample_period = 2000;
  pe_attr_sampling.precise_ip = 2;
  pe_attr_sampling.sample_type = PERF_SAMPLE_IP | PERF_SAMPLE_ADDR | PERF_SAMPLE_WEIGHT | PERF_SAMPLE_DATA_SRC;
  int memory_sampling_fd = perf_event_open(&pe_attr_sampling, 0, 9, -1, 0);
  if (memory_sampling_fd == -1) {
    printf("perf_event_open_failed: %s\n", strerror(errno));
    return -1;
  }
  long page_size = sysconf(_SC_PAGESIZE);
  size_t mmap_len = page_size + page_size * 1024;
  struct perf_event_mmap_page *metadata_page = mmap(NULL, mmap_len, PROT_WRITE, MAP_SHARED, memory_sampling_fd, 0);
  if (metadata_page == MAP_FAILED) {
    fprintf (stderr, "Couldn't mmap file descriptor: %s - errno = %d\n",
	     strerror (errno), errno);
    return -1;
  }   
  
  // Use libpfm to set attribute parameter for perf_event_open                                                                                                 
  /* pfm_perf_encode_arg_t pfm_arg; */
  /* struct perf_event_attr pe_attr_pfm; */
  /* memset(&pe_attr_pfm, 0, sizeof(pe_attr_pfm)); */
  /* memset(&pfm_arg, 0, sizeof(pfm_arg)); */
  /* pfm_arg.size = sizeof(pfm_arg); */
  /* pfm_arg.attr = &pe_attr_pfm; */
  /* char *str = "holds event string after call to pfm_get_os_event_encoding"; */
  /* pfm_arg.fstr = &str; */
  /* pfm_initialize(); */
  /* int pfm_encoding_res = pfm_get_os_event_encoding("UNC_QMC_NORMAL_READS", PFM_PLM0, PFM_OS_PERF_EVENT, &pfm_arg); */
  /* if (pfm_encoding_res != PFM_SUCCESS) { */
  /*   printf("pfm_get_os_event_encoding failed: %s\n", pfm_strerror(pfm_encoding_res)); */
  /*   return -1; */
  /* } */
  /* pe_attr_pfm.size = sizeof(struct perf_event_attr); */
  /* printf("config = %" PRIu64 ", config1 = %" PRIu64 ", config2 = %" PRIu64 ", type = %u, disabled = %d, inherit = %d, pinned = %d, exclusive = %d, exclude_user = %d, exclude_kernel = %d, exclude_hv = %d, exclude_idle = %d \n", pe_attr_pfm.config, pe_attr_pfm.config1, pe_attr_pfm.config2, pe_attr_pfm.type, pe_attr_pfm.disabled, pe_attr_pfm.inherit, pe_attr_pfm.pinned, pe_attr_pfm.exclusive, pe_attr_pfm.exclude_user, pe_attr_pfm.exclude_kernel, pe_attr_pfm.exclude_hv, pe_attr_pfm.exclude_idle); */

  // Starts measuring
  struct timeval t1, t2;
  double elapsedTime;
  gettimeofday(&t1, NULL);
  ioctl(memory_reads_fd, PERF_EVENT_IOC_RESET, 0);
  ioctl(memory_reads_fd, PERF_EVENT_IOC_ENABLE, 0);
  ioctl(memory_sampling_fd, PERF_EVENT_IOC_RESET, 0);
  ioctl(memory_sampling_fd, PERF_EVENT_IOC_ENABLE, 0);

  // Access memory
  read_memory(memory, size);

  // Stop measuring
  ioctl(memory_reads_fd, PERF_EVENT_IOC_DISABLE, 0);
  ioctl(memory_sampling_fd, PERF_EVENT_IOC_DISABLE, 0);
  
  // Print results
  int64_t memory_reads_count;
  read(memory_reads_fd, &memory_reads_count, sizeof(memory_reads_count));
  gettimeofday(&t2, NULL);
  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
  printf("time = %.3f ms\n", elapsedTime);
  printf("memory reads count = %" PRId64 "\n", memory_reads_count);
  uint64_t head = metadata_page->data_head;
  rmb();
  struct perf_event_header *header = (struct perf_event_header *)((char *)metadata_page + page_size);
  uint64_t i = 0;
  int memory_count = 0;
  int total_count = 0;
  while (i < head) {
    if (header -> type == PERF_RECORD_SAMPLE) {
      struct sample *sample = (struct sample *)((char *)(header) + 8);
      if (is_served_by_memory(sample->data_src)) {
	memory_count++;
      }
      total_count++;
    }
    i = i + header -> size;
    header = (struct perf_event_header *)((char *)header + header -> size);
  }
  printf("%d memory samples on %d samples\n", memory_count, total_count);  

  close(memory_reads_fd);
  close(memory_sampling_fd);
  free(memory);
  return 0;
}
