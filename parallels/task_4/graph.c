#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "graph.h"

graph_t *graph_generate(const int n, const int w) {
  int *weights = malloc(n * n * sizeof(int));
  assert(weights);

  graph_t *g = calloc(1, sizeof(graph_t));
  assert(g);
  g->n = n;
  g->weights = weights;

  for (int i = 0; i < n; i++) {
    weights[i * n + i] = -1;
    for (int j = i + 1; j < n; j++) {
      int weight = rand() % w + 1;
      weights[i * n + j] = weight;
      weights[j * n + i] = weight;
    }
  }

  return g;
}

inline int graph_weight(const graph_t *g, const int a, const int b) {
  if (a > g->n || b > g->n) {
    return -1;
  }
  return g->weights[a * g->n + b];
}

graph_t *graph_read(FILE *f) {
  int n;
  assert(fscanf(f, "%d", &n));
  int *weights = malloc(n * n * sizeof(int));
  assert(weights);

  graph_t *g = calloc(1, sizeof(graph_t));
  assert(g);
  g->n = n;
  g->weights = weights;
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      assert(fscanf(f, "%d", weights + i * n + j));
    }
  }

  // check graph for consistency
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      assert(weights[i * n + j] == weights[j * n + i]);
      if (i == j) {
        assert(weights[i * n + i] == -1);
      }
    }
  }

  return g;
}


graph_t *graph_read_file(const char *filename) {
  FILE *f = fopen(filename, "r");
  assert(f);
  graph_t *g = graph_read(f);
  fclose(f);
  return g;
}


void graph_dump(const graph_t *g, FILE *f) {
  assert(fprintf(f, "%d\n", g->n) > 0);
  for (int i = 0; i < g->n; i++) {
    for (int j = 0; j < g->n; j++) {
      assert(fprintf(f, "%d ", graph_weight(g, i, j)));
    }
    assert(fprintf(f, "\n"));
  }
}

void graph_dump_file(const graph_t *g, const char *filename) {
  FILE *f = fopen(filename, "w");
  assert(f);
  graph_dump(g, f);
  fclose(f);
}


void graph_destroy(graph_t *g) {
  free(g->weights);
  free(g);
}