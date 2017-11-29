#include <stdio.h>
#include <stdlib.h>

#include "graph.h"

int main(int argc, char **argv) {
  int n = atoi(argv[1]);
  graph_t *graph = graph_generate(n, 100);
  graph_dump_file(graph, "example.txt");
  return 0;
}