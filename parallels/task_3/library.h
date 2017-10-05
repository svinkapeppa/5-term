#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

/* ========== STRUCTURES ========== */

typedef struct ctx_t {
  int n;
  int m;
  int P;
  int *data;
  int *sorted;
  double worktime;
} ctx_t;

typedef struct arguments {
  ctx_t *first;
  int second;
} arguments;

/* ========== PROTOTYPES ========== */

void ctor(void *context);
void dtor(void *context);
void statistics(void *context);
void work(void *context);
void sort(void *context);
void *routine(void *args);
int cmpfunc(const void *a, const void *b);

/* ========== IMPLEMENTATIONS ========== */

void ctor(void *context) {
  int i;
  ctx_t *ctx;

  ctx = context;

  ctx->data = calloc(ctx->n, sizeof(int));
  assert(ctx->data);

  ctx->sorted = calloc(ctx->n, sizeof(int));
  assert(ctx->sorted);

  srand(time(NULL));

  for (i = 0; i < ctx->n; ++i) {
    ctx->data[i] = rand() % 1000;
    ctx->sorted[i] = ctx->data[i];
  }
}

void statistics(void *context) {
  int i;
  FILE *fd;
  ctx_t *ctx;

  ctx = context;

  fd = fopen("stats.txt", "w+");

  if (fd == NULL) {
    fprintf(stderr, "FAILED TO PRINT STATISTICS\n");
    exit(100500);
  }

  fprintf(fd, "%fs %d %d %d\n", ctx->worktime, ctx->n, ctx->m, ctx->P);

  fclose(fd);

  fd = fopen("data.txt", "w+");

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

void dtor(void *context) {
  ctx_t *ctx;

  ctx = context;

  statistics(ctx);

  free(ctx->data);
  free(ctx->sorted);
}

void work(void *context) {
  ctx_t *ctx;
  struct timeval start, end;

  ctx = context;

  assert(gettimeofday(&start, NULL) == 0);

  sort(ctx);

  assert(gettimeofday(&end, NULL) == 0);

  ctx->worktime = ((end.tv_sec - start.tv_sec) * 1000000u + \
                    end.tv_usec - start.tv_usec) / 1.e6;
}

void sort(void *context) {
  int i;
  ctx_t *ctx;

  ctx = context;

  pthread_t threads[ctx->P];
  arguments args[ctx->P];

  for (i = 0; i < ctx->P; ++i) {
    args[i].first = ctx;
    args[i].second = i;
  }

  for (i = 0; i < ctx->P; ++i) {
    pthread_create(&threads[i], NULL, &routine, (void *)&args[i]);
  }

  for (i = 0; i < ctx->P; ++i) {
    pthread_join(threads[i], NULL);
  }
}

void *routine(void *args_) {
  int index, left, right, i;
  int *buf;
  ctx_t *ctx;
  arguments *args;

  args = args_;

  ctx = args->first;
  index = args->second; 

  left = ctx->m * index;
  right = ctx->m * (index + 1);

  if (right > ctx->n) {
    right = ctx->n;
  }

  while(left < ctx->n) {
    buf = calloc(right - left, sizeof(int));
    assert(buf);

    for (i = 0; i < right - left; ++i) {
      buf[i] = ctx->sorted[i + left];
    }

    qsort(buf, right - left, sizeof(int), cmpfunc);

    for (i = 0; i < right - left; ++i) {
      ctx->sorted[i + left] = buf[i];
    }

    left += ctx->m * ctx->P;
    right += ctx->m * ctx->P;

    if (right > ctx->n) {
      right = ctx->n;
    }

    free(buf);
  }
}

int cmpfunc(const void *a, const void *b) {
  return (*(int *)a - *(int *)b);
}