#ifndef FIFO_H
#define FIFO_H

#include <pthread.h>

typedef struct {
  int head;
  int size;
  int *data;
  pthread_rwlock_t rwlock;
} fifo_t;

void *fifo_init(fifo_t *fifo, int size);
void fifo_push(fifo_t *fifo, int token);
int fifo_pop(fifo_t *fifo);

#endif
