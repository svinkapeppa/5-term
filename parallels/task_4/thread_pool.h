#ifndef THREAD_POOL
#define THREAD_POOL

#include <pthread.h>

#include "random.h"

typedef struct task_t {
  void (* func) (void *, random_data_t *);
  void *arg;
} task_t;

typedef struct thread_pool_t {
  int alive;
  int thread_number;
  int inactive;
  int task_size;
  int task_capacity;
  task_t *tasks;
  pthread_t *threads;
  pthread_cond_t pending;
  pthread_cond_t gather;
  pthread_mutex_t lock;
} thread_pool_t;

void thread_pool_init(thread_pool_t *tp, int size, int capacity);
int thread_pool_put(thread_pool_t *tp, task_t task);
void thread_pool_barrier(thread_pool_t *tp);
void thread_pool_destroy(thread_pool_t *tp);

#endif