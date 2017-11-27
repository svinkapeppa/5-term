#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "library.h"

void experiment(void *context) {
  ctx_t *ctx = context;
  int seed = getseed(ctx->rank, ctx->size);

  int *buf = getdata(ctx, &seed);

  MPI_File fd;
  MPI_File_open(MPI_COMM_WORLD, "data.bin",
                MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &fd);
  MPI_Datatype type;
  MPI_Type_vector(ctx->l, ctx->l * ctx->size, ctx->l * ctx->size * ctx->a,
                  MPI_INT, &type);
  MPI_Type_commit(&type);

  int row = ctx->rank / ctx->a;
  int column = ctx->rank % ctx->a;
  int offset = (row * ctx->a * ctx->l * ctx->l * ctx->size +\
                column * ctx->l * ctx->size) * sizeof(int);

  MPI_File_set_view(fd, offset, MPI_INT, type, "native", MPI_INFO_NULL);
  MPI_File_write(fd, buf, ctx->l * ctx->l * ctx->size,
                 MPI_INT, MPI_STATUS_IGNORE);
  MPI_Type_free(&type);
  MPI_File_close(&fd);

  free(buf);
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
  
  free(buf);
  
  return seed;
}

int *getdata(void *context, int *seed) {
  ctx_t *ctx = context;

  int *buf = calloc(ctx->size * ctx->l * ctx->l, sizeof(int));
  assert(buf);

  for (int i = 0; i < ctx->N; ++i) {
    int x = rand_r(seed) % ctx->l;
    int y = rand_r(seed) % ctx->l;
    int offset = rand_r(seed) % ctx->size;
    buf[(y * ctx->l + x) * ctx->size + offset] += 1;
  }

  return buf;
}

void stats(void *context, double workingtime) {
  ctx_t *ctx = context;

  if (ctx->rank == MASTER) {
    FILE* fd = fopen("stats.txt", "w+");
    if (fd == NULL) {
      fprintf(stderr, "SOMETHING BAD HAPPENED\n");
      exit(128);
    } else {
      fprintf(fd, "%d %d %d %d %0.2f\n",
              ctx->l, ctx->a, ctx->b, ctx->N, workingtime);
      fclose(fd);
    }
  }
}