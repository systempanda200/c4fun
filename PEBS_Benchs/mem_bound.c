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

#define CPU 9
#define NUMA_NODE 1

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
    } else if (data_src.mem_lvl & PERF_MEM_LVL_UNC) {
      return 1;
    } else {
      return 0;
    }
  } else {
    return 0;
  }
}

int is_served_by_remote_cache(union perf_mem_data_src data_src) {         
  if (data_src.mem_lvl & PERF_MEM_LVL_HIT) {
    if (data_src.mem_lvl & PERF_MEM_LVL_REM_CCE1) {
      return 1; 
    } else if (data_src.mem_lvl & PERF_MEM_LVL_REM_CCE2) {
      printf("fuckkk\n");
      return 1; 
    } else {
      return 0;
    }
  } else {
    return 0;
  }
}

char* concat(const char *s1, const char *s2) {
  char *result = malloc(strlen(s1) + strlen(s2) + 1);
  if (result == NULL) {
    return "malloc failed in concat\n";
  }
  strcpy(result, s1);
  strcat(result, s2);
  return result;
}

char * get_data_src_opcode(union perf_mem_data_src data_src) {
  char * res = concat("", "");
  char * old_res;
  if (data_src.mem_op & PERF_MEM_OP_NA) {
    old_res = res;
    res = concat(res, "NA");
    free(old_res);
  }
  if (data_src.mem_op & PERF_MEM_OP_LOAD) {
    old_res = res;
    res = concat(res, "Load");
    free(old_res);
  }
  if (data_src.mem_op & PERF_MEM_OP_STORE) {
    old_res = res;
    res = concat(res, "Store");
    free(old_res);
  }
  if (data_src.mem_op & PERF_MEM_OP_PFETCH) {
    old_res = res;
    res = concat(res, "Prefetch");
    free(old_res);
  }
  if (data_src.mem_op & PERF_MEM_OP_EXEC) {
    old_res = res;
    res = concat(res, "Exec code");
    free(old_res);
  }
  return res;
}

char * get_data_src_level(union perf_mem_data_src data_src) {
  char * res = concat("", "");
  char * old_res;
  if (data_src.mem_lvl & PERF_MEM_LVL_NA) {
    old_res = res;
    res = concat(res, "NA");
    free(old_res);
  }
  if (data_src.mem_lvl & PERF_MEM_LVL_L1) {
    old_res = res;
    res = concat(res, "L1");
    free(old_res);
  } else if (data_src.mem_lvl & PERF_MEM_LVL_LFB) {
    old_res = res;
    res = concat(res, "LFB");
    free(old_res);
  } else if (data_src.mem_lvl & PERF_MEM_LVL_L2) {
    old_res = res;
    res = concat(res, "L2");
    free(old_res);
  } else if (data_src.mem_lvl & PERF_MEM_LVL_L3) {
    old_res = res;
    res = concat(res, "L3");
    free(old_res);
  } else if (data_src.mem_lvl & PERF_MEM_LVL_LOC_RAM) {
    old_res = res;
    res = concat(res, "Local RAM");
    free(old_res);
  } else if (data_src.mem_lvl & PERF_MEM_LVL_REM_RAM1) {
    old_res = res;
    res = concat(res, "Remote RAM 1 hop");
    free(old_res);
  } else if (data_src.mem_lvl & PERF_MEM_LVL_REM_RAM2) {
    old_res = res;
    res = concat(res, "Remote RAM 2 hops");
    free(old_res);
  } else if (data_src.mem_lvl & PERF_MEM_LVL_REM_CCE1) {
    old_res = res;
    res = concat(res, "Remote Cache 1 hop");
    free(old_res);
  } else if (data_src.mem_lvl & PERF_MEM_LVL_REM_CCE2) {
    old_res = res;
    res = concat(res, "Remote Cache 2 hops");
    free(old_res);
  } else if (data_src.mem_lvl & PERF_MEM_LVL_IO) {
    old_res = res;
    res = concat(res, "I/O Memory");
    free(old_res);
  } else if (data_src.mem_lvl & PERF_MEM_LVL_UNC) {
    old_res = res;
    res = concat(res, "Uncached Memory");
    free(old_res);
  }
  if (data_src.mem_lvl & PERF_MEM_LVL_HIT) {
    old_res = res;
    res = concat(res, " Hit");
    free(old_res);
  } else if (data_src.mem_lvl & PERF_MEM_LVL_MISS) {
    old_res = res;
    res = concat(res, " Miss");
    free(old_res);
  }
  return res;
}

/**
 * Structure representing a sample gathered with the library in sampling mode.
 */
struct sample {
  uint64_t ip;
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

  uint64_t *memory;
  if (numa_available() == -1) {
    memory = malloc(size);
  } else {
    memory = numa_alloc_onnode(size, NUMA_NODE);
  }
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
  uint64_t *memory;
  if (numa_available() == -1) {
    memory = malloc(size);
  } else {
    memory = numa_alloc_onnode(size, NUMA_NODE);
  }
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
  printf ("Usage %s size access_mode period\n\tsize is the size of the allocated and accessed memory in mega bytes\n\taccess_mode is either seq for sequential accesses or rand for random accesses\n\tperiod the sampling period in number of events\n", prog_name);
}

int main(int argc, char **argv) {

  /**
   * Check and get arguments.
   */
  if (argc <= 3) {
    usage(argv[0]);
    return -1;
  }
  size_t size = atol(argv[1]) * 1000000;
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
  uint64_t period = atol(argv[3]);
  printf("Running on %lu mega bytes\n", (size / 1000000));

  /**
   * Allocates and fills memory. Because the memory is filled, all its
   * pages are touched and as a consequence, no page faults will occur
   * during the measurement.
   */
  uint64_t *memory;
  numa_available();
  if (access_mode == access_seq) {
    memory = alloc_fill_memory_sequential(size);
  } else {
    memory = alloc_fill_memory_rand(size);
  }

  /**
   * Profile memory
   */

  // Manualy set and open ram memory counting event
  struct perf_event_attr pe_attr_unc_memory;
  memset(&pe_attr_unc_memory, 0, sizeof(pe_attr_unc_memory));
  pe_attr_unc_memory.size = sizeof(pe_attr_unc_memory);
  pe_attr_unc_memory.type = 6; // /sys/bus/event_source/uncore/type
  pe_attr_unc_memory.config = 0x072c; // QMC_NORMAL_READS.ANY
  pe_attr_unc_memory.disabled = 1;
  int memory_reads_fd = perf_event_open(&pe_attr_unc_memory, -1, CPU, -1, 0);
  if (memory_reads_fd == -1) {
    printf("perf_event_open_failed: %s\n", strerror(errno));
    return -1;
  }

  // Manualy set and open load retired counting event
  struct perf_event_attr pe_attr_loads;
  memset(&pe_attr_loads, 0, sizeof(pe_attr_loads));
  pe_attr_loads.size = sizeof(pe_attr_loads);
  pe_attr_loads.type = PERF_TYPE_RAW;
  pe_attr_loads.config = 0x010b; // MEM_INST_RETIRED.LOADS
  pe_attr_loads.disabled = 1;
  pe_attr_loads.exclude_kernel = 1;
  int loads_fd = perf_event_open(&pe_attr_loads, 0, CPU, -1, 0);
  if (loads_fd == -1) {
    printf("perf_event_open_failed: %s\n", strerror(errno));
    return -1;
  }

  // Manualy set and open instructions retired counting event
  struct perf_event_attr pe_attr_inst;
  memset(&pe_attr_inst, 0, sizeof(pe_attr_inst));
  pe_attr_inst.size = sizeof(pe_attr_inst);
  pe_attr_inst.type = PERF_TYPE_RAW;
  pe_attr_inst.config = 0x00c0; // INST_RETIRED.ANY
  pe_attr_inst.disabled = 1;
  pe_attr_inst.exclude_kernel = 1;
    pe_attr_inst.exclude_hv = 1;
  int inst_fd = perf_event_open(&pe_attr_inst, 0, CPU, -1, 0);
  if (inst_fd == -1) {
    printf("perf_event_open_failed: %s\n", strerror(errno));
    return -1;
  }

  // Manualy set and open memory sampling event
  struct perf_event_attr pe_attr_sampling;
  memset(&pe_attr_sampling, 0, sizeof(pe_attr_sampling));
  pe_attr_sampling.size = sizeof(pe_attr_sampling);
  pe_attr_sampling.type = PERF_TYPE_RAW;
  pe_attr_sampling.config = 0x100b; // MEM_INST_RETIRED.LATENCY_ABOVE_THRESHOLD
  pe_attr_sampling.disabled = 1;
  pe_attr_sampling.config1 = 3; // latency threshold
  pe_attr_sampling.sample_period = period;
  pe_attr_sampling.precise_ip = 2;
  pe_attr_sampling.sample_type = PERF_SAMPLE_IP | PERF_SAMPLE_ADDR | PERF_SAMPLE_WEIGHT | PERF_SAMPLE_DATA_SRC;
  int memory_sampling_fd = perf_event_open(&pe_attr_sampling, 0, CPU, -1, 0);
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
  
  // Starts measuring
  struct timeval t1, t2;
  double elapsedTime;
  gettimeofday(&t1, NULL);
  ioctl(memory_reads_fd, PERF_EVENT_IOC_RESET, 0);
  ioctl(memory_reads_fd, PERF_EVENT_IOC_ENABLE, 0);
  ioctl(memory_sampling_fd, PERF_EVENT_IOC_RESET, 0);
  ioctl(memory_sampling_fd, PERF_EVENT_IOC_ENABLE, 0);
  ioctl(loads_fd, PERF_EVENT_IOC_RESET, 0);
  ioctl(loads_fd, PERF_EVENT_IOC_ENABLE, 0);
  ioctl(inst_fd, PERF_EVENT_IOC_RESET, 0);
  ioctl(inst_fd, PERF_EVENT_IOC_ENABLE, 0);
  
  // Access memory
  read_memory(memory, size);

  // Stop measuring
  ioctl(inst_fd, PERF_EVENT_IOC_DISABLE, 0);
  ioctl(loads_fd, PERF_EVENT_IOC_DISABLE, 0);
  ioctl(memory_sampling_fd, PERF_EVENT_IOC_DISABLE, 0);
  ioctl(memory_reads_fd, PERF_EVENT_IOC_DISABLE, 0);

  // Print results
  printf("\nMemory is between %" PRIx64 " and %" PRIx64 "\n", (uint64_t)memory, ((uint64_t)memory + size));
  int64_t memory_reads_count, loads_count, insts_count;
  read(memory_reads_fd, &memory_reads_count, sizeof(memory_reads_count));
  read(loads_fd, &loads_count, sizeof(loads_count));
  read(inst_fd, &insts_count, sizeof(insts_count));
  gettimeofday(&t2, NULL);
  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
  printf("\n");
  printf("%-60s = %15.3f \n",       "time               (               milliseconds         )", elapsedTime);
  printf("%-60s = %15" PRId64 "\n", "ram reads count    (uncore event: QMC_NORMAL_READS.ANY  )", memory_reads_count);
  printf("%-60s = %15" PRId64 "\n", "instructions count (  core event: INST_RETIRED.ANY      )", insts_count);
  printf("%-60s = %15" PRId64 "\n", "loads count        (  core event: MEM_INST_RETIRED.LOADS)", loads_count);
  uint64_t head = metadata_page->data_head;
  //printf("sampling head = %" PRIu64 "\n", head);
  //printf("mmap size = %ld \n", mmap_len);
  rmb();
  struct perf_event_header *header = (struct perf_event_header *)((char *)metadata_page + page_size);
  uint64_t i = 0;
  int remote_cache_count = 0;
  int memory_count = 0;
  int total_count = 0;
  int in_malloced_count = 0;
  printf("\n");
  while (i < head) {
    if (header -> type == PERF_RECORD_SAMPLE) {
      struct sample *sample = (struct sample *)((char *)(header) + 8);
      printf("Memory sample ");
      printf("IP %-20" PRIx64, sample -> ip);
      printf("@%-20" PRIx64, sample -> addr);
      if (sample->addr >= (uint64_t)memory && sample->addr <= (uint64_t)memory + size) {
	in_malloced_count++;
	printf("%-10s", "in");
      } else {
	printf("%-10s", "out");
      }
      printf("Weight = %-10" PRIu64, sample -> weight);
      char *op = get_data_src_opcode(sample -> data_src);
      printf("%s", op);
      free(op);
      char *level = get_data_src_level(sample -> data_src);
      printf(", %s", level);
      free(level);
      printf("\n");
      if (is_served_by_memory(sample->data_src)) {
	memory_count++;
      } else if (is_served_by_remote_cache(sample->data_src)) {
	remote_cache_count++;
      }
      total_count++;
    }
    i = i + header -> size;
    header = (struct perf_event_header *)((char *)header + header -> size);
  }
  printf("\n");
  printf("%d memory samples on %d samples (%.3f%%)\n", memory_count, total_count, (memory_count / (float) total_count * 100));  
  printf("%d samples in malloced memory on %d samples (%.3f%%)\n", in_malloced_count, total_count, (in_malloced_count / (float) total_count * 100));  
  printf("%d remote cache samples on %d samples (%.3f%%)\n\n", remote_cache_count, total_count, (remote_cache_count / (float) total_count * 100));  
  close(memory_reads_fd);
  close(memory_sampling_fd);
  if (numa_available() == -1) {
    free(memory);
  } else {
    numa_free(memory, size);
  }
  return 0;
}
