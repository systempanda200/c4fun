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
#include <perfmon/pfmlib_perf_event.h>
#include <err.h>
#include <numa.h>

#include "numemp.h"

int nb_numa_nodes;
int pfm_init_res;
int pfm_encoding_res;

__attribute__((constructor)) void init(void) {

  // Get numa configuration
  int available = numa_available();
  if (available == -1) {
    nb_numa_nodes = -1;
  } else {
    nb_numa_nodes = numa_num_configured_nodes();
  }

  // Initializes pfm
  pfm_init_res = pfm_initialize();
}

const char * numemp_error_message(int error) {
  switch(error) {
  case ERROR_NUMA_LIB :
    return "libnumemp: numa lib not available";
    break;
  case ERROR_PFM_INIT :
    return pfm_strerror(pfm_init_res);
    break;
  case ERROR_PFM_ENCODE :
    return pfm_strerror(pfm_encoding_res);
    break;
  case ERROR_PERF_EVENT_OPEN :
    return strerror(errno);
    break;
  default:
    return "libnumemp: unknown error";
  }
}

int numemp_start(struct numemp_measure *measure) {

  // Check everything is ok
  if(nb_numa_nodes == -1) {
    return ERROR_NUMA_LIB;
  }
  if(pfm_init_res != PFM_SUCCESS) {
    return ERROR_PFM_INIT;
  }

  // Use libpfm to get attribute parameter for perf_event_open from event string
  measure->nb_nodes = nb_numa_nodes;
  pfm_perf_encode_arg_t arg;
  struct perf_event_attr attr;
  memset(&attr, 0, sizeof(struct perf_event_attr));

  memset(&arg, 0, sizeof(arg));
  arg.size = sizeof(pfm_perf_encode_arg_t);
  arg.attr = &attr;
  char *fstr = "this string will hold the complete event string after call to pfm_get_os_event_encoding";
  arg.fstr = &fstr;
  pfm_encoding_res = pfm_get_os_event_encoding("UNC_QMC_NORMAL_READS", PFM_PLM3, PFM_OS_PERF_EVENT, &arg);
  //pfm_encoding_res = pfm_get_os_event_encoding("INSTRUCTIONS", PFM_PLM0 | PFM_PLM3, PFM_OS_PERF_EVENT, &arg);
  attr.size = sizeof(struct perf_event_attr);
  if (pfm_encoding_res != PFM_SUCCESS) {
    return ERROR_PFM_ENCODE;
  }
  printf("Fully qualified event string: %s\n", *arg.fstr);

  // Open the event with Linux system call
  measure->fd = perf_event_open(&attr, 0, -1, -1, 0);
  if(measure->fd == -1) {
    return ERROR_PERF_EVENT_OPEN;
  }

  // Starts measure
  ioctl(measure->fd, PERF_EVENT_IOC_RESET, 0);
  ioctl(measure->fd, PERF_EVENT_IOC_ENABLE, 0);

  return 0;
}

void numemp_stop(struct numemp_measure *measure) {
  ioctl(measure->fd, PERF_EVENT_IOC_DISABLE, 0);
  read(measure->fd, &measure->nodes_bandwidth[0], sizeof(long long));
  close(measure->fd);
}
