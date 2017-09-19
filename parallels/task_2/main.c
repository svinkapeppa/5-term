#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#include "runner.h"

typedef struct ctx_t {
  int n;
  int m;
  int P;
  int *data;
  int *sorted;
  double work_time;
} ctx_t;

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
    ctx->data[i] = rand();
    ctx->sorted[i] = ctx->data[i];
  }
}

void dtor(void *context) {
  ctx_t *ctx;

  ctx = context;

  free(ctx->data);
  free(ctx->sorted);
}

int bin_search(int *a, int key, int left, int right) {
   int mid;

   if (right - left <= 1) {
    return right;
  } else {
    mid = (left + right) / 2;

    if (a[mid] < key) {
      return bin_search(a, key, mid, right);
    } else {
      return bin_search(a, key, left, mid);
    }
  }
}

void sort(int *a, int left, int right) {
  int i, j, tmp;

  for (i = left + 1; i < right; ++i) {
    tmp = a[i];
    j = i - 1;
    while (j >= left && a[j] > tmp) {
      a[j + 1] = a[j];
      --j;
    }
    a[j + 1] = tmp;
  }
}

/*
  ! x and y - left and right bounds of the 'left' array
  ! m and n - same for the 'right' array
*/
int * merge(int *a, int x, int y, int m, int n) {
  int i, size;
  int *tmp;

  size = (n - m) + (y - x);

  tmp = calloc(size, sizeof(int));
  assert(tmp);

  i = 0;

  while(1) {
    if (x == y) {
      while (m != n) {
        tmp[i] = a[m];
        ++i;
        ++m;
      }

      return tmp;
    }

    if (m == n) {
      while (x != y) {
        tmp[i] = a[x];
        ++i;
        ++x;
      }

      return tmp;
    }

    if (a[x] < a[m]) {
      tmp[i] = a[x];
      ++x;
    } else {
      tmp[i] = a[m];
      ++m;
    }

    ++i;
  }
}

void migrate(int *contestant, int *sorted, int left, int right) {
  int i;

  for (i = left; i < right; ++i) {
    contestant[i] = sorted[i - left];
  }
}

void turn(void *context, int *a, int left, int right) {
  int half, median, right_index, size;
  int *sortedleft;
  int *sortedright;
  ctx_t *ctx;

  ctx = context;

  half = (left + right) / 2;
  median = (left + half) / 2;

  if ((right - left) <= ctx->m) {
    sort(a, left, right);
    return;
  }

  #pragma omp parallel
  {
    #pragma omp single
    {
      #pragma omp task
      turn(ctx, a, left, half);
      #pragma omp task
      turn(ctx, a, half, right);
    }
    right_index = bin_search(a, a[median], half, right);
    #pragma omp single
    {
      #pragma omp task
      sortedleft = merge(a, left, median, half, right_index);
      #pragma omp task
      sortedright = merge(a, median, half, right_index, right);
      #pragma omp task
      size = (right_index - half) + (median - left);
    }
    #pragma omp single
    {
      #pragma omp task
      migrate(a, sortedleft, left, left + size);
      #pragma omp task
      migrate(a, sortedright, left + size, right);
    }
  }

  free(sortedleft);
  free(sortedright);
}

void work(void *context) {
  int i;
  ctx_t *ctx;

  ctx = context;

  turn(ctx, ctx->sorted, 0, ctx->n);

  for (i = 0; i < ctx->n; ++i) {
    printf("%d\n", ctx->sorted[i]);
  }
}

int main(int argc, char **argv) {
  int n, m, P;

  n = atoi(argv[1]);
  m = atoi(argv[2]);
  P = atoi(argv[3]);

  omp_set_num_threads(P);

  ctx_t ctx = {
    .n = n,
    .m = m,
    .P = P
  };

  runner_pre(ctor, &ctx);

  runner_run(work, &ctx, "parallel sort");

  runner_post(dtor, &ctx);

  return 0;
}