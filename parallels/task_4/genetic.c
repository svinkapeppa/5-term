#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "graph.h"
#include "genetic.h"
#include "random.h"
#include "thread_pool.h"

route_t tsp(int population_size, int criteria,
            thread_pool_t *tp, random_data_t *data) {
  int counter = 0;
  int selection_size = (int) (population_size * SELECTION_RATE);
  route_t *population = calloc(population_size, sizeof(route_t));
  args_t *args = calloc(selection_size, sizeof(args_t));
  assert(population);
  assert(args);
  
  route_t best = {
    .route = malloc(graph->n * sizeof(int)),
    .fitness = INT_MAX
  };

  for (int i = 0; i < selection_size; ++i) {
    args[i].buffer = calloc(graph->n, sizeof(int));
    assert(args[i].buffer);
  }

  for (int i = 0; i < population_size; ++i) {
    task_t task = {
      .func = generate_route,
      .arg = population + i
    };
    population[i].route = calloc(graph->n, sizeof(int));
    assert(population[i].route);
    thread_pool_put(tp, task);
  }
  thread_pool_barrier(tp);
  update_best(&best, population, population_size);

  FILE *fd;
  fd = fopen("data.txt", "w+");
  if (fd == NULL) {
    fprintf(stderr, "UNABLE TO PRINT DATA\n");
    exit(128);
  }

  while (counter++ < criteria) {
    selection_stage(population, selection_size, population_size, data);
    crossover_stage(population, selection_size, population_size,
                    tp, args, data);
    mutation_stage(population, population_size, tp, data);
   
    if (update_best(&best, population, population_size)) {
      counter = 0;
    } 

    int min = INT_MAX;
    int max = 0;
    double mean = 0;

    for (int i = 0; i < population_size; ++i) {
      if (min > population[i].fitness) {
        min = population[i].fitness;
      }
      if (max < population[i].fitness) {
        max = population[i].fitness;
      }
      mean += (double) population[i].fitness / population_size;
    }

    fprintf(fd, "%d %d %f\n", min, max, mean);
  }

  for (int i = 0; i < selection_size; ++i) {
    free(args[i].buffer);
  }
  for (int i = 0; i < population_size; ++i) {
    free(population[i].route);
  }
  free(args);
  free(population);
  fclose(fd);

  return best;
}

void generate_route(void *arg, random_data_t *data) {
  route_t *r = arg;

  for (int i = 0; i < graph->n; ++i) {
    r->route[i] = i;
  }

  for (int i = 0; i < graph->n; ++i) {
    int j = i + rnd(data) % (graph->n - i);
    int tmp = r->route[i];
    r->route[i] = r->route[j];
    r->route[j] = tmp;
  }

  calculate_fitness(r, data);
}

void calculate_fitness(void *arg, random_data_t *data) {
  route_t *r = arg;

  r->fitness = graph_weight(graph, r->route[graph->n - 1], r->route[0]);
  for (int i = 1; i < graph->n; ++i) {
    r->fitness += graph_weight(graph, r->route[i - 1], r->route[i]);
  }
}

void mutate(void *arg, random_data_t *data) {
  route_t *r = arg;
  int i = rnd(data) % graph->n;
  int j = rnd(data) % graph->n;

  r->fitness -= vertex_weight(r, i);
  r->fitness -= vertex_weight(r, j);

  int tmp = r->route[i];
  r->route[i] = r->route[j];
  r->route[j] = tmp;

  r->fitness += vertex_weight(r, i);
  r->fitness += vertex_weight(r, j);
}

void crossover(void *arg_, random_data_t *data) {
  args_t *arg = arg_;
  route_t *first = arg->first;
  route_t *second = arg->second;
  route_t *child = arg->child;
  int *used = arg->buffer;

  int start = rnd(data) % graph->n;
  int len = rnd(data) % graph->n;
  int i, j;

  memset(used, 0, graph->n * sizeof(int));
  for (i = 0; i < len; ++i) {
    child->route[i] = first->route[(start + i) % graph->n];
    used[child->route[i]] = 1;
  }

  j = 0;
  while (i < graph->n) {
    while (used[second->route[j]]) {
      ++j;
    }
    child->route[i++] = second->route[j++];
  }

  calculate_fitness(child, data);
}

int update_best(route_t *best, route_t *candidates, int size) {
  int new_best = -1;

  for (int i = 0; i < size; ++i) {
    if (candidates[i].fitness < best->fitness) {
      best->fitness = candidates[i].fitness;
      new_best = i;
    }
  }

  if (new_best >= 0) {
    memcpy(best->route, candidates[new_best].route, graph->n * sizeof(int));
    new_best = -1;
    return 1;
  } else {
    return 0;
  }
}

int vertex_weight(route_t *r, int i) {
  int prev;
  if (i == 0) {
    prev = graph->n - 1;
  } else {
    prev = i - 1;
  }
  int next = (i + 1) % graph->n;
  return graph_weight(graph, r->route[i], r->route[next])
       + graph_weight(graph, r->route[prev], r->route[i]);
}

void dump_array(FILE *fd, int *array, int n) {
  for (int i = 0; i < n - 1; ++i)
    fprintf(fd, "%d ", array[i]);
  fprintf(fd, "%d\n", array[n - 1]);
}

void selection_stage(route_t *population, int selection_size,
                     int population_size, random_data_t *data) {
  for (int i = 1; i < selection_size; ++i) {
    int u = rnd(data) % (population_size - i);
    int v = rnd(data) % (population_size - i);

    int index;
    if (population[u].fitness < population[v].fitness) {
      index = v;
    } else {
      index = u;
    }

    route_t tmp = population[population_size - i - 1];
    population[population_size - i - 1] = population[index];
    population[index] = tmp;
  } 
}

void crossover_stage(route_t *population, int selection_size,
                     int population_size, thread_pool_t *pool,
                     args_t *args, random_data_t *data) {
  for (int i = 0; i < selection_size; ++i) {
    int u = rnd(data) % (population_size - selection_size);
    int v = rnd(data) % (population_size - selection_size);
    task_t task = {
      .func = crossover,
      .arg = args + i
    };

    args[i].first = population + u;
    args[i].second = population + v;
    args[i].child = population + population_size - i - 1;

    thread_pool_put(pool, task);
  }
  thread_pool_barrier(pool);
}

void mutation_stage(route_t *population, int population_size,
                    thread_pool_t *pool, random_data_t *data) {
  for (int i = 0; i < population_size; ++i) {
    if ((double) rnd(data) / RAND_MAX < MUTATION_RATE) {
      task_t task = {
        .func = mutate,
        .arg = population + i
      };
      thread_pool_put(pool, task);
    }
  }
  thread_pool_barrier(pool); 
}