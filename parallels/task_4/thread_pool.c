#include <pthread.h>
#include <stdlib.h>

#include "thread_pool.h"

void *work(void *arg) {
  thread_pool_t *tp = arg;
  random_data_t data;

  random_data_init(&data);

  pthread_mutex_lock(&tp->lock);
  while(1) {
    if (++tp->inactive == tp->thread_number) {
      pthread_cond_signal(&tp->gather);
    }

    while ((tp->alive == 1) && (tp->task_size > 0)) {
      pthread_cond_wait(&tp->pending, &tp->lock);
    }

    if (tp->task_size > 0) { 
      task_t task = tp->tasks[--tp->task_size];
      --tp->inactive;
      pthread_mutex_unlock(&tp->lock);
      task.func(task.arg, &data);
      pthread_mutex_lock(&tp->lock);
    } else {
      break;
    }
  }
  pthread_mutex_unlock(&tp->lock);

  random_data_destroy(&data);

  return NULL;
}

void thread_pool_init(thread_pool_t *tp, int *size, int *capacity) {
  tp->alive = 1;
  tp->thread_number = *size;
  tp->inactive = 0;
  tp->task_size = 0;
  tp->task_capacity = *capacity;
  tp->tasks = calloc(tp->task_capacity, sizeof(task_t));
  tp->threads = calloc(tp->thread_number, sizeof(pthread_t));
  assert(tp->tasks);
  assert(tp->threads);
  pthread_cond_init(&tp->pending, NULL);
  pthread_cond_init(&tp->gather, NULL);
  pthread_mutex_init(&tp->lock, NULL);

  for (int i = 0; i < tp->thread_number; ++i) {
    pthread_create(&tp->threads[i], NULL, work, tp);
  }
}

int thread_pool_put(thread_pool_t *tp, task_t *task) {
  pthread_mutex_lock(&tp->lock);
  if (tp->alive == 0) {
    pthread_mutex_unlock(&tp->lock);
    return 0;
  }

  if (tp->task_size == tp->task_capacity) {
    tp->task_capacity *= 2;
    tp->tasks = realloc(tp->tasks, tp->task_capacity * sizeof(task_t));
  }

  tp->tasks[tp->task_size++] = task;

  pthread_cond_signal(&tp->pending);
  pthread_mutex_unlock(&tp->lock);

  return 0;
}

void thread_pool_barrier(thread_pool_t *tp) {
  pthread_mutex_lock(&tp->lock);
  while (tp->inactive < tp->thread_number || tp->task_size > 0) {
    pthread_cond_wait(&tp->gather, &tp->lock);
  }
  pthread_mutex_unlock(&tp->lock);
}

void thread_pool_destroy(thread_pool_t *tp) {
  pthread_mutex_lock(&tp->lock);
  tp->alive = 0;
  pthread_cond_broadcast(&tp->task);
  pthread_mutex_unlock(&tp->lock);

  for (int i = 0; i < thread_number; ++i) {
    pthread_join(tp->threads[i], NULL);
  }
  pthread_cond_destroy(&tp->pending);
  pthread_cond_destroy(&tp->gather);
  pthread_mutex_destroy(&tp->lock);
  free(tp->threads);
  free(tp->tasks);
}