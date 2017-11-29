#ifndef GENETIC_H
#define GENETIC_H

#include <stdio.h>

#include "graph.h"
#include "random.h"
#include "thread_pool.h"

#define ARRAY_SIZE 655360
#define MUTATION_RATE 0.1
#define SELECTION_RATE 0.75

typedef struct route_t {
  int *route;
  int fitness;
} route_t;

typedef struct args_t {
  route_t *first;
  route_t *second;
  route_t *child;
  int *buffer;
} args_t;

graph_t *graph;

route_t tsp(int population_size, int stopping_time,
            thread_pool_t *pool, random_data_t *data);
void generate_route(void *arg, random_data_t *data);
void calculate_fitness(void *arg, random_data_t *data);
void mutate(void *arg, random_data_t *data);
void crossover(void *arg_, random_data_t *data);
int update_best(route_t *best, route_t *candidates, int size);
void dump_array(FILE *fd, int *array, int n);
int vertex_weight(route_t *r, int i);
void selection_stage(route_t *population, int selection_size,
                     int population_size, random_data_t *data);
void crossover_stage(route_t *population, int selection_size,
                     int population_size, thread_pool_t *pool,
                     args_t *args, random_data_t *data);
void mutation_stage(route_t *population, int population_size,
                    thread_pool_t *pool, random_data_t *data);

#endif