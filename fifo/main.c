#include "fifo.h"
#include <stdio.h>

int main()
{
  fifo_t f;
  fifo_init(&f, 10);
  for (int i = 0; i < 10; i++){
    fifo_push(&f, i);
  }
  for (int i = 0; i < 10; i++){
    printf("Poping %d\n", fifo_pop(&f));
  }
  return 0;
}
