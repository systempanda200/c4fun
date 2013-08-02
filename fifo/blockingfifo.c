#include "fifo.h"
#include <stdio.h>
#include <stdlib.h>

void* fifo_init(fifo_t *fifo, int size)
{
  fifo -> size = size;
  fifo -> data = malloc(sizeof(int) * size);
  fifo -> head = -1;
  pthread_rwlock_init(&fifo -> rwlock, NULL);
  return fifo -> data;
}

void fifo_push(fifo_t *fifo, int elem)
{
  pthread_rwlock_wrlock(&fifo -> rwlock);
  ++(fifo -> head);
  if (fifo -> head > fifo -> size -1) {
    printf("Can't push in a full FIFO\n");
    exit(EXIT_FAILURE);
  }
  *(fifo -> data + fifo -> head) = elem;
  pthread_rwlock_unlock(&fifo -> rwlock);
}

int fifo_pop(fifo_t *fifo)
{
  pthread_rwlock_rdlock(&fifo -> rwlock);
  if (fifo -> head < 0) {
    printf("Can't pop an empty FIFO\n");
    exit(EXIT_FAILURE);
  }
  int pop = *(fifo -> data + fifo -> head);
  --(fifo -> head);
  pthread_rwlock_unlock(&fifo -> rwlock);
  return pop;
}
