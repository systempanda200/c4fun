#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <perfmon/pfmlib_perf_event.h>

#define rmb()		asm volatile("lfence" ::: "memory")

struct sample {
  uint64_t ip;
  uint32_t pid;
  uint32_t tid;
  uint64_t addr;
  uint32_t cpuid;
  uint64_t period;
};

struct mmap_sample {
  uint32_t pid;
  uint32_t tid;
  uint64_t addr;
  uint64_t len;
  uint64_t pgoff;
  char filename[100];
};

char * get_sample_type_name(int type) {
  switch (type) {
  case PERF_RECORD_SAMPLE:
    return "PERF_RECORD_SAMPLE";
  case PERF_RECORD_MMAP:
    return "PERF_RECORD_MMAP";
  }
  return NULL;
}

int main() {

  /* Declarations */
  struct perf_event_attr pe;
  size_t page_size;
  int fd;
  int length;

  /* Get thread and process id from linux kernel */
  long tid = syscall(SYS_gettid);
  pid_t pid = getpid();
  printf("Process id = %d\n", pid);
  printf("Thread  id = %ld\n", tid);

  memset(&pe, 0, sizeof(struct perf_event_attr));
  pe.size = sizeof(struct perf_event_attr);
  pe.type = PERF_TYPE_RAW;
  pe.config1 = 3;
  pe.config = 0x100b;
  pe.sample_period = 1;
  pe.sample_type = PERF_SAMPLE_IP | PERF_SAMPLE_TID | PERF_SAMPLE_ADDR | PERF_SAMPLE_CPU | PERF_SAMPLE_PERIOD | PERF_SAMPLE_WEIGHT | PERF_SAMPLE_DATA_SRC;
  pe.precise_ip = 2;
  pe.mmap = 1;
  pe.task = 1;
  pe.disabled = 1;
  pe.exclude_kernel = 1;
  pe.exclude_hv = 1;

  pid = 0;
  int cpu = -1;
  printf("Calling perf event open: sampling period = %lu, pid = %d, cpu = %d, use freq = %u, config = %lu, sample_type= %lu\n",
		  pe.sample_period, pid, cpu, pe.freq, pe.config, pe.sample_type);

  /* printf("Calling syscal with sampling period = %lu, pid = %d, cpu = %d, pe.type=%u, pe.config=%lu, pe.config1=%lu\n", */
  /* 	 pe.sample_freq, pid, cpu, pe.type, pe.config, pe.config1); */

  fd = perf_event_open(&pe, pid, cpu, -1, 0);
  if (fd == -1) {
    fprintf(stderr, "Error in perf_event_open: %s\n", strerror (errno));
    exit(EXIT_FAILURE);
  }

  /* mmap the file descriptor returned by perf_event_open to read the samples */
  page_size = (size_t) sysconf (_SC_PAGESIZE);
  length = page_size + page_size * 64;
  struct perf_event_mmap_page *metadata_page = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0);
  if (metadata_page == MAP_FAILED) {
    fprintf (stderr, "Couldn't mmap file descriptor: %s - errno = %d\n",
	     strerror (errno), errno);
    exit (EXIT_FAILURE);
  }

  /* Perform recording */
  int size = 1000;
  int * data = (int *)malloc(size * sizeof(int));
  printf("Fifo Addr is between %p and %p\n", (void *)data, (void *)(data + size * sizeof(int)));
  printf("Recording samples\n");
  ioctl(fd, PERF_EVENT_IOC_RESET, 0);
  ioctl(fd, PERF_EVENT_IOC_ENABLE, 0); // Start sampling

  for (int i = 0; i < size; i++){
    int d = data[i];
    data[i] = d + 1;
  }

  /* sleep(0); */
  /* sleep(0); */
  /* sleep(0); */
  /* sleep(0); */
  /* int fdCode = open("/home/manu/numa-eval/NuMemP/libnumemp.so", O_RDWR); */
  /* void *code = mmap(NULL, length, PROT_EXEC, MAP_SHARED, fdCode, 0); */
  /* if (code == MAP_FAILED) { */
  /* 	fprintf (stderr, "Couldn't mmap /home/manu/numa-eval/NuMemP/libnumemp.so: %s - errno = %d\n", */
  /* 		 strerror (errno), errno); */
  /* 	exit (EXIT_FAILURE); */
  /* } */
  /* printf("Yess\n"); */

  ioctl(fd, PERF_EVENT_IOC_DISABLE, 0); // STOP sampling

  /* Reading result */
  /* printf("Metada page version = %d\n", metadata_page -> version); */
  /* printf("Metada page compat_version = %d\n", metadata_page -> compat_version); */
  /* printf("Metada page time_enabled = %l" PRIu64 "\n", metadata_page -> time_enabled); */
  /* printf("Metada page time_running = %l" PRIu64 "\n", metadata_page -> time_running); */

  /* Head is updated by the kernel and indicates where is the last
     produced sample. Tail must be updated by user code to indicate
     to the kernel where we are in reading events in order to not
     override the buffer */
  uint64_t head = metadata_page -> data_head;
  printf("Metada page head = %" PRIu64 "\n", head);
  uint64_t tail = metadata_page -> data_tail;
  printf("Metada page tail = %" PRIu64 "\n", tail);
  rmb();

  // we need a char * to have pointer arithmetic working on bytes.
  struct perf_event_header *header = (struct perf_event_header *)((char *)metadata_page + page_size);
  uint64_t i = 0;
  while (i < head % length) {
    //printf("\nEvent type = %s\n", get_sample_type_name(header -> type));
    //printf("Event size = %" PRIu16 "\n", header -> size);
    if (header -> type == PERF_RECORD_SAMPLE) {
      struct sample *sample = (struct sample *)((char *)(header) + 8);
      /* printf("Sample details:\n"); */
      /* printf("  Instruction pointer = %" PRIx64 "\n", sample -> ip); */
      /* printf("  Process id = %u\n", sample -> pid); */
      /* printf("  Thread id = %u\n", sample -> tid); */
      printf("  Addr = %" PRIx64 " by %" PRIx64 " is in FIFO = %s\n", sample -> addr, sample -> ip, ((sample -> addr > (uint64_t)data && sample -> addr < (uint64_t)data + size * sizeof(int)) ? "true" : "false"));
      //printf("  Cpu id = %u\n", sample -> cpuid);
      //printf("  Sampling period = %" PRIu64 "\n", sample -> period);
    } else if (header -> type == PERF_RECORD_MMAP) {
      struct mmap_sample *sample = (struct mmap_sample *)((char *)(header) + 8);
      printf("Sample details:\n");
      printf("  Process id = %u\n", sample -> pid);
      printf("  Thread id = %u\n", sample -> tid);
      printf("  Addr = %" PRIx64 "\n", sample -> addr);
      printf("  Length = %" PRIx64 "\n", sample -> len);
      printf("  pgoff = %" PRIx64 "\n", sample -> pgoff);
      printf("  file name = %s\n", sample -> filename);
    }
    i = i + header -> size;
    header = (struct perf_event_header *)((char *)header + header -> size);
    /* printf("i = %" PRIu64 " < head = %" PRIu64 " ? %s\n", i, head, ((i < head) ? "true" : "false")); */
  }

  /* munmap - close */
  if (munmap(metadata_page, length)) {
    fprintf (stderr, "Couldn't unmmap file descriptor: %s - errno = %d\n",
	     strerror (errno), errno);
    exit (EXIT_FAILURE);
  }
  close(fd);

  /* printf("Measuring instruction count\n"); */

  /* ioctl(fd, PERF_EVENT_IOC_RESET, 0); */
  /* ioctl(fd, PERF_EVENT_IOC_ENABLE, 0); */

  /* // To measure */

  /* ioctl(fd, PERF_EVENT_IOC_DISABLE, 0); */
  /* read(fd, &count, sizeof(long long)); */
  /* close(fd); */
  /* printf("Used %lld instructions\n", count); */

  return 0;
}
