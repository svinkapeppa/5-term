#include <assert.h>
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

#define MASTER 0
#define UP 8
#define DOWN 2
#define LEFT 4
#define RIGHT 6

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
  int *seed;
} ctx_t;

typedef struct walker_t {
  int step;
  int x;
  int y;
} walker_t;

int max(double up, double down, double left, double right);
int step(void *context, int rank, int size);
void walk(void *context, int rank, int size);

int main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int nodenumber = atoi(argv[2]) * atoi(argv[3]);
  int *seed = calloc(nodenumber, sizeof(int));
  assert(seed);

  srand(time(NULL));
  for (int i = 0; i < nodenumber; ++i) {
    seed[i] = rand();
  }

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
    .seed = seed
  };

  walk(&ctx, rank, size);

  MPI_Finalize();
  return 0;
}

int max(double up, double down, double left, double right)
{
  if (up >= down && up >= left && up >= right) {
    return UP; 
  } else if (down >= up, down >= left, down >= right) {
    return DOWN;
  } else if (left >= up, left >= down, left >= right) {
    return LEFT;
  } else {
    return RIGHT;
  }
}

int step(void *context, int rank, int size)
{
  ctx_t *ctx = context;

  double bidderup = ctx->up * rand_r(&ctx->seed[rank]);
  double bidderdown = ctx->down * rand_r(&ctx->seed[rank]);
  double bidderleft = ctx->left * rand_r(&ctx->seed[rank]);
  double bidderright = ctx->right * rand_r(&ctx->seed[rank]);

  return max(bidderup, bidderdown, bidderleft, bidderright);
}

void walk(void *context, int rank, int size)
{
  ctx_t *ctx = context;
  int dst = rank % size;
  int v = step(ctx, rank, size);
  printf("rank %d: direction %d\n", dst, v);
}