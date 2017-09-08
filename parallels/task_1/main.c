#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#include "runner.h"

typedef struct ctx_t {
  int *d;
  int *s;
  int a;
  int b;
  int x;
  double p;
  size_t N;
  int P;
  double boundary;
  int res;
  int steps;
  double time;
} ctx_t;

void ctor(void *context) {
  size_t i;

  ctx_t *ctx;
  ctx = context;

  ctx->d = calloc(ctx->N, sizeof(int));
  assert(ctx->d);

  ctx->s = calloc(ctx->N, sizeof(int));
  assert(ctx->s);

  for (i = 0; i < ctx->N; i++) {
    ctx->d[i] = ctx->x;
  }

  for (i = 0; i < ctx->N; i++) {
    ctx->s[i] = 0;
  }

  ctx->boundary = (1 - ctx->p) * ((double)RAND_MAX);
}

void dtor(void *context) {
  ctx_t *ctx;

  ctx = context;

  free(ctx->d);
  free(ctx->s);
}

int model(void *context, int i, int extra) {
  int seed, decision;
  double turn;
  ctx_t *ctx;

  seed = (i + extra) ^ omp_get_thread_num();
  ctx = context;

  while ((ctx->d[i] != ctx->a) && (ctx->d[i] != ctx->b)) {
    turn = (double)rand_r(&seed);
    if (turn > ctx->boundary) {
      decision = 1;
    } else {
      decision = -1;
    }
    ctx->d[i] += decision;
    ctx->s[i] += 1;
  }

  if (ctx->d[i] == ctx->b) {
    return 1;
  } else {
    return 0;
  }
}

void statistics(void *context) {
  FILE *fp;
  ctx_t *ctx;

  ctx = context;

  fp = fopen("stats.txt", "w+");
  fprintf(fp, "%.2f %.2f %.4fs %d %d %d %ld %.2f %d\n",
          ((double) ctx->res) / ((double) ctx->N),
          ((double) ctx->steps) / ((double) ctx->N),
          ctx->time, ctx->a, ctx->b, ctx->x,
          ctx->N, ctx->p, ctx->P);
}

void run(void *context) {
  size_t i, j;
  int sum, steps;
  double worktime;
  ctx_t *ctx;
  struct timeval start, end;

  sum = 0;
  steps = 0;
  ctx = context;

  srand(time(NULL));
  int extra = rand() % 1000;

  assert(gettimeofday(&start, NULL) == 0);

  #pragma omp parallel for reduction (+ : sum, steps)
  for (i = 0; i < ctx->N; i++) {
    sum += model(ctx, i, extra);
    steps += ctx->s[i];
  }

  assert(gettimeofday(&end, NULL) == 0);

  worktime = ((end.tv_sec - start.tv_sec) * 1000000u + \
              end.tv_usec - start.tv_usec) / 1.e6;

  ctx->res = sum;
  ctx->steps = steps;
  ctx->time = worktime;

  statistics(ctx);
}

int main(int argc, char **argv) {
  int a, b, x, P;
  double p;
  size_t N;

  a = atoi(argv[1]);
  b = atoi(argv[2]);
  x = atoi(argv[3]);
  N = atoi(argv[4]);
  p = atof(argv[5]);
  P = atoi(argv[6]);

  omp_set_num_threads(P);
  
  ctx_t ctx = {
      .a = a,
      .b = b,
      .x = x,
      .p = p,
      .N = N,
      .P = P
  };

  runner_pre(ctor, &ctx);
  
  runner_run(run, &ctx, "test");

  runner_post(dtor, &ctx);

  return 0;
}