#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "queue.h"

queue_t *queue_init() {
  queue_t *q = calloc(1, sizeof(queue_t));
  assert(q);
  q->len = 0;
  q->cap = MIN_QUEUE_SIZE;
  q->data = calloc(1, q->cap * sizeof(void *));
  assert(q->data);
  pthread_mutex_init(&q->lock, NULL);
  return q;
}

void queue_destroy(queue_t *q) {
  pthread_mutex_destroy(&q->lock);
  free(q->data);
  free(q);
}

static void queue_resize(queue_t *q, const int size) {
  void *ptr = calloc(size, sizeof(void *));
  assert(ptr);
  q->len -= q->head;
  memcpy(ptr, q->data + q->head, q->len * sizeof(void *));
  free(q->data);
  q->cap = size;
  q->head = 0;
  q->data = ptr;
}

void queue_push(queue_t *q, void *x) {
  pthread_mutex_lock(&q->lock);
  if (q->len == q->cap) {
    queue_resize(q, q->cap << 1);
  }
  q->data[q->len++] = x;
  pthread_mutex_unlock(&q->lock);
}

void *queue_pop(queue_t *q) {
  pthread_mutex_lock(&q->lock);
  assert(q->len >= q->head);
  if (q->len - q->head == 0) {
    pthread_mutex_unlock(&q->lock);
    return NULL;
  }
  if (q->len - q->head < q->cap >> 1) {
    queue_resize(q, q->cap >> 1);
  }
  void *ret = q->data[q->head++];
  pthread_mutex_unlock(&q->lock);
  return ret;
}

int queue_len(queue_t *q) {
  pthread_mutex_lock(&q->lock);
  int ret = q->len - q->head;
  pthread_mutex_unlock(&q->lock);
  return ret;
}