#ifndef RANDOM_H
#define RANDOM_H

typedef struct random_data_t {
  int *array;
  int last;
} random_data_t;

void random_init(int seed, int size, int capacity);
void random_data_init(random_data_t *data);
void random_destroy();
void random_data_destroy(random_data_t *data);
int rnd(random_data_t *data);

#endif