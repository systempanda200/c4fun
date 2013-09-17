#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/syscall.h>

#define rmb()		asm volatile("lfence" ::: "memory")

long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
		     int cpu, int group_fd, unsigned long flags) {
    int ret;

    ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
		  group_fd, flags);
    return ret;
}

struct sample {
    uint64_t ip;
    uint32_t pid;
    uint32_t tid;
    uint32_t cpuid;
};

int main() {

    /* Declarations */
    struct perf_event_attr pe;
    size_t page_size;
    int fd;
    int length;
    void *buffer;
    struct perf_event_mmap_page *metadata_page;
    long tid;
    pid_t pid;

    /* Get thread and process id from linux kernel */
    tid = syscall(SYS_gettid);
    pid = getpid();
    printf("Process id = %d\n", pid);
    printf("Thread  id = %ld\n", tid);

    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = PERF_TYPE_HARDWARE;
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_INSTRUCTIONS;
    pe.sample_period = 100;
    pe.sample_type = PERF_SAMPLE_IP | PERF_SAMPLE_TID | PERF_SAMPLE_CPU;
    pe.precise_ip = 2;
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;

    fd = perf_event_open(&pe, 0, -1, -1, 0);
    if (fd == -1) {
	fprintf(stderr, "Error in perf_event_open \n");
	exit(EXIT_FAILURE);
    }

    /* mmap the file descriptor returned by perf_event_open to read the samples */
    page_size = (size_t) sysconf (_SC_PAGESIZE);
    length = page_size + page_size * 64;
    buffer = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0);
    if (buffer == MAP_FAILED) {
	fprintf (stderr, "Couldn't mmap file descriptor: %s - errno = %d\n",
		 strerror (errno), errno);
	exit (EXIT_FAILURE);
    }

    /* Perform recording */
    printf("Recording samples\n");
    ioctl(fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
    sleep(0);
    ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);

    /* Reading result */
    metadata_page = buffer;
    printf("Metada page version = %d\n", metadata_page -> version);
    printf("Metada page compat_version = %d\n", metadata_page -> compat_version);
    printf("Metada page time_enabled = %l" PRIu64 "\n", metadata_page -> time_enabled);
    printf("Metada page time_running = %l" PRIu64 "\n", metadata_page -> time_running);
    int head = metadata_page -> data_head;
    rmb();

    struct perf_event_header *header = (struct perf_event_header *)(buffer + page_size);
    printf("First event type = %d\n", header -> type);
    printf("First event size = %d\n", header -> size);
    if (header -> type == PERF_RECORD_SAMPLE) {
	struct sample *sample = (struct sample *)(buffer + page_size + 8);
	printf("Sample details:\n");
	printf("  Instruction pointer = %0x\n", sample -> ip);
	printf("  Process id = %u\n", sample -> pid);
	printf("  Thread id = %u\n", sample -> tid);
	printf("  Cpu id = %u\n", sample -> cpuid);
    }

    /* munmap - close */
    if (munmap(buffer, length)) {
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
