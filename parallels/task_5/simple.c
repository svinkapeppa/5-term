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
} ctx_t;

void stats(void *context);

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
    .down = atof(argv[9])
  };

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