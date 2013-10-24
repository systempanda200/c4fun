#define MAX_NB_NUMA_NODES

#define ERROR_NUMA_LIB   -1
#define ERROR_PFM_INIT   -2
#define ERROR_PFM_ENCODE -3


struct numemp_measure {
  long fd;
  int nb_nodes;
  long long nodes_bandwidth[MAX_NB_NUMA_NODES];
};

int numemp_start(struct numemp_measure *measure);
void numemp_stop(struct numemp_measure *measure);
const char * numemp_error_message(int error);
