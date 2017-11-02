#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MASTER 0

typedef struct ctx_t {
  int l;
  int a;
  int b;
  int n;
  int N;
  double up;
  double down;
  double left;
  double right;
  int rank;
  int size;
} ctx_t;

typedef struct point_t {
  int x;
  int y;
  int iteration;
} point_t;

void stats(void *context);
void experiment(void *context);
int getseed(int rank, int size);

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  ctx_t ctx = {
    .l = atoi(argv[1]),
    .a = atoi(argv[2]),
    .b = atoi(argv[3]),
    .n = atoi(argv[4]),
    .N = atoi(argv[5]),
    .left = atof(argv[6]),
    .right = atof(argv[7]),
    .up = atof(argv[8]),
    .down = atof(argv[9]),
    .rank = rank,
    .size = size
  };

  experiment(&ctx);

  if (rank == MASTER) {
    stats(&ctx);
  }

  MPI_Finalize();
  return 0;
}

void stats(void *context) {
  ctx_t *ctx = context;
  FILE *fd;

  fd = fopen("stats.txt", "w+");

  if (fd == NULL) {
    fprintf(stderr, "Unable to print stats\n");
    exit(128);
  } else {
    fprintf(fd, "%d %d %d %d %d %.2f %.2f %.2f %.2f\n",
            ctx->l, ctx->a, ctx->b, ctx->n, ctx->N,
            ctx->left, ctx->right, ctx->up, ctx->down);
  }

  fclose(fd);
}

void experiment(void *context) {
  ctx_t *ctx = context;
  int seed = getseed(ctx->rank, ctx->size);

  srand(seed);

  point_t *points = calloc(ctx->N, sizeof(point_t));
  int *pointsnumber = calloc(1, sizeof(int));
  assert(points);
  assert(pointsnumber);
  *pointsnumber = ctx->N;
  for (int i = 0; i < ctx->N; ++i) {
    points[i].x = rand() % ctx->l;
    points[i].y = rand() % ctx->l;
    points[i].iteration = ctx->n;
  }

  free(pointsnumber);
  free(points);
}

int getseed(int rank, int size) {
  int *buf = NULL;
  int seed;

  if (rank == MASTER) {
    srand(time(NULL));
    buf = calloc(size, sizeof(int));
    assert(buf);
    for (int i = 0; i < size; ++i) {
      buf[i] = rand();
    }
  }

  MPI_Scatter(buf, 1, MPI_INT, &seed, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
  
  if (buf != NULL) {
    free(buf);
  }
  
  return seed;
}