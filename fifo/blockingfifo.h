#ifndef FIFO_H
#define FIFO_H

#include <pthread.h>

typedef struct {
  int head;
  int size;
  int *data;
  pthread_rwlock_t rwlock;
} blocking_fifo_t;

void *fifo_init(blocking_fifo_t *fifo, int size);
void fifo_push(blocking_fifo_t *fifo, int token);
int fifo_pop(blocking_fifo_t *fifo);

#endif
