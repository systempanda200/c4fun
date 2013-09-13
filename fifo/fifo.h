#ifndef FIFO_H
#define FIFO_H

typedef struct {
  int head;
  int size;
  int *data;
} fifo_t;

void *fifo_init(fifo_t *fifo, int size);
void fifo_push(fifo_t *fifo, int token);
int fifo_pop(fifo_t *fifo);

#endif
