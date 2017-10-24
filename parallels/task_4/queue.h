#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>

#define MIN_QUEUE_SIZE 1

typedef struct queue_t {
  pthread_mutex_t lock;
  int head;
  int cap;
  int len;
  void **data;
} queue_t;

queue_t *queue_init();
void queue_destroy(queue_t *q);
void queue_push(queue_t *q, void *x);
void *queue_pop(queue_t *q);
int queue_len(queue_t *q);

#endif