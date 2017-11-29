#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "graph.h"
#include "random.h"
#include "thread_pool.h"
#include "genetic.h"

int main(int argc, char **argv) {
  thread_pool_t tp;
  random_data_t data;
  route_t solution;
  struct timeval start, end;

  int thread_number = atoi(argv[1]);
  int population = atoi(argv[2]);
  int criteria = atoi(argv[3]);

  if (!strcmp(argv[4], "--generate")) {
    srand(time(NULL));
    graph = graph_generate(atoi(argv[5]), 100);
  } else {
    graph = graph_read_file(argv[5]);
  }

  random_init(time(NULL) + 1, ARRAY_SIZE, thread_number + 2);
  random_data_init(&data);
  thread_pool_init(&tp, thread_number, population);

  assert(gettimeofday(&start, NULL) == 0);
  solution = tsp(population, criteria, &tp, &data);
  assert(gettimeofday(&end, NULL) == 0);

  double working_time = ((end.tv_sec - start.tv_sec) * 1000000u +\
                          end.tv_usec - start.tv_usec) / 1.e6;
  
  FILE *fd;
  fd = fopen("stats.txt", "w");
  if (fd == NULL) {
    fprintf(stderr, "UNABLE TO PRINT STATS\n");
    exit(128);
  } else {
    fprintf(fd, "%d %d %d %.2fs %d\n", thread_number, population,
            criteria, working_time, solution.fitness);
    dump_array(fd, solution.route, graph->n);
  }
  fclose(fd);

  free(solution.route);
  thread_pool_destroy(&tp);
  random_data_destroy(&data);
  random_destroy();
  graph_destroy(graph);

  return 0;
}