#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define NB_ELEMS 10000000 // 10 megas
#define SIZE  NB_ELEMS * sizeof(int) // 10 * 4 = 40 megas

int rand_range(int min, int max) {
  int diff = max - min;
  return (int) (((double)(diff+1)/RAND_MAX) * rand() + min);
}

int main(int argc, char **argv) {
  int * data = (int *)malloc(SIZE);
  while (1) {
    for (int i = 0; i < NB_ELEMS; i++){
      int r = rand_range(0, NB_ELEMS -1);
      int d = data[r];
      data[r] = d + 1;
    }
  }
  free(data);
  return 0;
}
