#include "numemp.h"
#include <stdio.h>
#include <unistd.h>

int main() {
  struct numemp_measure m;
  int res = numemp_start(&m);
  if(res < 0) {
    printf("numemp_start error : %s\n", numemp_error_message(res));
    return -1;
  }
  sleep(5);
  numemp_stop(&m);
  for(int i = 0; i < m.nb_nodes; i++) {
    printf("Memory bandwidth for node %d is %lld\n", i, m.nodes_bandwidth[i]);
  }

  res = numemp_start(&m);
  if(res < 0) {
    printf("Error : %s\n", numemp_error_message(res));
    return -1;
  }
  sleep(15);
  numemp_stop(&m);
  for(int i = 0; i < m.nb_nodes; i++) {
    printf("Memory bandwidth for node %d is %lld\n", i, m.nodes_bandwidth[i]);
  }

  return 0;
}
