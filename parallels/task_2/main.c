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
    return left;
  } else {
    mid = (left + right) / 2;

    if (a[mid] < key) {
      bin_search(a, key, mid, right);
    } else {
      bin_search(a, key, left, mid);
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

void left_merge(int *final, int *left, int *right, int lind, int rind) {
  int i, j, k;

  i = j = k = 0;

  while(1) {
    if (left[i] < right[j]) {
      final[k] = left[i];
      ++i;
    } else {
      final[k] = right[j];
      ++j;
    }

    ++k;

    if (i == lind) {
      while (j != rind) {
        final[k] = right[j];
        ++k;
        ++j;
        return;
      }
    }

    if (j == rind) {
      while (i != lind) {
        final[k] = left[i];
        ++k;
        ++i;
        return;
      }
    }
  }
}

void right_merge(int *final, int *left, int *right, int lind,
                 int lind2, int rind, int rind2) {
  int i, j, k;

  i = lind;
  j = rind;
  k = lind + rind - 1;

  while(1) {
    if (left[i] < right[j]) {
      final[k] = left[i];
      ++i;
    } else {
      final[k] = right[j];
      ++j;
    }

    ++k;

    if (i == lind2) {
      while (j != rind2) {
        final[k] = right[j];
        ++k;
        ++j;
        return;
      }
    }

    if (j == rind2) {
      while (i != lind2) {
        final[k] = left[i];
        ++k;
        ++i;
        return;
      }
    }
  }
}

void turn(void *context, int *a, int left, int right) {
  int half, median, right_index;
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
    #pragma omp single nowait
    {
      #pragma omp task
      turn(ctx, a, left, half);
      #pragma omp task
      turn(ctx, a, half, right);
    }
    #pragma omp single nowait
    {
      #pragma omp task
      right_index = bin_search(a, a[median], half, right);
    }
    // #pragma omp single nowait
    // {
    //   #pragma omp task
    //   left_merge(sorted_final, sorted_left, sorted_right, left_index, right_index);
    //   #pragma omp task
    //   right_merge(sorted_final, sorted_left, sorted_right, left_index, 
    //               half-left_index, right_index, len-half-right_index);
    // }
  }
}

void work(void *context) {
  ctx_t *ctx;

  ctx = context;

  turn(ctx, ctx->sorted, 0, ctx->n);
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