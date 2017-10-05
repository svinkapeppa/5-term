#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "runner.h"

typedef struct ctx_t {
  int n;
  int *data;
  int *sorted;
  double worktime;
} ctx_t;

void ctor(void *context) {
  size_t i;
  ctx_t *ctx;

  ctx = context;

  ctx->data = calloc(ctx->n, sizeof(int));
  assert(ctx->data);

  ctx->sorted = calloc(ctx->n, sizeof(int));
  assert(ctx->sorted);

  for (i = 0; i < ctx->n; i++) {
    ctx->data[i] = rand();
    ctx->sorted[i] = ctx->data[i];
  }
}

int cmpfunc(const void *a, const void *b) {
  return (*(int *)a - *(int *)b);
}

void statistics(void *context) {
  int i;
  FILE *fd;
  ctx_t *ctx;

  ctx = context;

  fd = fopen("seqstats.txt", "w+");

  if (fd == NULL) {
    fprintf(stderr, "FAILED TO PRINT STATISTICS\n");
    exit(100500);
  }

  fprintf(fd, "%fs %d\n", ctx->worktime, ctx->n);

  fclose(fd);

  fd = fopen("seqdata.txt", "w+");

  if (fd == NULL) {
    fprintf(stderr, "FAILED TO PRINT DATA\n");
    exit(100500);
  }

  for (i = 0; i < ctx->n; ++i) {
    fprintf(fd, "%d ", ctx->data[i]);
  }

  fprintf(fd, "\n");

  for (i = 0; i < ctx->n; ++i) {
    fprintf(fd, "%d ", ctx->sorted[i]);
  }

  fclose(fd);
}

void run(void *context) {
  ctx_t *ctx;
  struct timeval start, end;

  ctx = context;

  assert(gettimeofday(&start, NULL) == 0);

  qsort(ctx->sorted, ctx->n, sizeof(int), cmpfunc);

  assert(gettimeofday(&end, NULL) == 0);

  ctx->worktime = ((end.tv_sec - start.tv_sec) * 1000000u + \
                    end.tv_usec - start.tv_usec) / 1.e6;
}

void dtor(void *context) {
  ctx_t *ctx;

  ctx = context;

  statistics(ctx);

  free(ctx->data);
  free(ctx->sorted);
}

int main(int argc, char **argv) {
  int n;

  n = atoi(argv[1]);
  
  ctx_t ctx = {
      .n = n
  };

  srand(time(NULL));

  runner_pre(ctor, &ctx);
  
  runner_run(run, &ctx, "test");

  runner_post(dtor, &ctx);

  return 0;
}