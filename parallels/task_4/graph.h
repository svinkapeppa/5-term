#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>

typedef struct graph_t {
  int n;
  int *weights;
} graph_t;


// graph_generate generates fully connected graph with n nodes without self-loops
// weight of edge between a pair of nodes is generated in range [1:w]
// standard rand() function is used to generate random weight,
// it is user responsiblity to init random with a propper seed
graph_t *graph_generate(const int n, const int w);

// graph_destroy frees all resources associated with the graph
void graph_destroy(graph_t *g);

// graph_weight returns weight of the edge (a,b)
// if edge doesn't exist than -1 will be returned
int graph_weight(const graph_t *g, const int a, const int b);

// graph_read read graph from a given file
// first line of file should contain a single number n (number of nodes)
// the following n lines represent adjacency matrix of the graph, where
// elements of matrix are the weights of edges for corresponding nodes
// -1 is used to denote self-loop
graph_t *graph_read(FILE *f);
graph_t *graph_read_file(const char *filename);

void graph_dump(const graph_t *g, FILE *f);
void graph_dump_file(const graph_t *g, const char *filename);

#endif