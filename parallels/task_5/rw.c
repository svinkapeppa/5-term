#include <assert.h>
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

#define MASTER 0
#define UP 8
#define DOWN 2
#define LEFT 4
#define RIGHT 6

typedef struct walker_t {
  int step;
  int x;
  int y;
} walker_t;

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
  walker_t *walker;
} ctx_t;

int max(double up, double down, double left, double right);
int step(void *context, int rank, int size);
void experiment(void *context, int rank, int size);
void init(void *context, int rank, int size);
void destroy(void *context, int rank, int size);
void walk(void *context, int rank, int size);

int main(int argc, char **argv)
{
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
    .down = atof(argv[9])
  };

  init(&ctx, rank, size);
  experiment(&ctx, rank, size);
  destroy(&ctx, rank, size);

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

  double bidderup = ctx->up * rand();
  double bidderdown = ctx->down * rand();
  double bidderleft = ctx->left * rand();
  double bidderright = ctx->right * rand();

  return max(bidderup, bidderdown, bidderleft, bidderright);
}

void experiment(void *context, int rank, int size)
{
  ctx_t *ctx = context;

  for (int i = 0; i < ctx->N; ++i) {
    walk(ctx, rank, size);
  }
}

void init(void *context, int rank, int size)
{
  ctx_t *ctx = context;
  int nodenumber = ctx->a * ctx->b;

  ctx->seed = calloc(nodenumber, sizeof(int));
  assert(ctx->seed);

  srand(time(NULL));
  for (int i = 0; i < nodenumber; ++i) {
    ctx->seed[i] = rand();\
  }
}

void walk(void *context, int rank, int size)
{
  return;
}

void destroy(void *context, int rank, int size)
{
  ctx_t *ctx = context;
  free(ctx->seed);
}