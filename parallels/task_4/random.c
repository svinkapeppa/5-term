#include <stdlib.h>
#include <pthread.h>

#include "random.h"

int random_alive, random_seed, random_size,
    random_last, random_max_capacity;
int **random_arrays;
pthread_t random_thread;
pthread_mutex_t random_lock;
pthread_cond_t random_cond_full, random_cond_empty;

void *generate(void *arg) {
  pthread_mutex_lock(&random_lock);
  while (1) {
    while ((random_alive == 1) && (random_last >= random_max_capacity)) {
      pthread_cond_wait(&random_cond_full, &random_lock);
    }

    if (random_alive == 0) {
      break;
    }

    pthread_mutex_unlock(&random_lock);
    int *array = calloc(random_size, sizeof(int));
    assert(array);
    for (int i = 0; i < random_size; ++i) {
      array[i] = rand_r(&random_seed);
    }
    pthread_mutex_lock(&random_lock);

    if (random_last == 0) {
      pthread_cond_signal(&random_cond_empty);
    }

    random_arrays[random_last++] = array;
  }
  pthread_mutex_unlock(&random_lock);

  return NULL;
}

void random_init(int *seed, int *size, int *capacity) {
  random_alive = 1;
  random_seed = *seed;
  random_size = *size;
  random_last = 0;
  random_max_capacity = *capacity;
  random_lock = PTHREAD_MUTEX_INITIALIZER;
  random_cond_full = PTHREAD_COND_INITIALIZER;
  random_cond_empty = PTHREAD_COND_INITIALIZER;
  random_arrays = calloc(random_max_capacity, sizeof(int *));
  assert(random_arrays); 

  pthread_create(&random_thread, NULL, generate, NULL);
}

void random_data_init(random_data_t *data) {
  data->last = random_size;
  data->array = NULL;
}

void random_destroy() {
  pthread_mutex_lock(&random_lock);
  random_alive = 0;
  for (int i = 0; i < random_last; ++i) {
    free(random_arrays[i]);
  }
  free(random_arrays);
  pthread_mutex_unlock(&random_lock);
  pthread_cond_signal(&random_cond_full);
  pthread_join(random_thread, NULL);
  pthread_mutex_destroy(&random_lock);
  pthread_cond_destroy(&random_cond_full);
  pthread_cond_destroy(&random_cond_empty);
}

void random_data_destroy(random_data_t *data) {
  free(data->array);
}

int random(random_data_t *data) {
  if (data->last == random_size) {
    data->last = 0;
    free(data->array);

    pthread_mutex_lock(&random_lock);
    while (random_last == 0) {
      pthread_cond_wait(&random_cond_empty, &random_lock);
    }
    data->array = random_arrays[--random_last];
    pthread_mutex_unlock(&random_lock);

    pthread_cond_signal(&random_cond_full);
  }

  return data->array[data->last++];
}