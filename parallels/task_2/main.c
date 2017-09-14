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

  srand(time(NULL));

  for (i = 0; i < ctx->n; ++i) {
    ctx->data[i] = rand();
  }
}

void dtor(void *context) {
  ctx_t *ctx;

  ctx = context;

  free(ctx->data);
  free(ctx->sorted);
}

int bin_search(int *array, int edge, int right_bound) {
  int left, right, mid;

  left = 0;
  right =  right_bound - 1;

  mid = (left + right) / 2;

  if (left == right) {
    return left;
  } else {
    if (array[mid] < edge) {
      bin_search(array, mid, right);
    } else {
      bin_search(array, left, mid);
    }
  }
}

int * sort(int *a, int len) {
  int i, j, tmp;
  int *sorted;

  sorted = calloc(len, sizeof(int));
  assert(sorted);

  for (i = 0; i < len; ++i) {
    sorted[i] = a[i];
  }

  for (i = 1; i < len; ++i) {
    for (j = i; j > 0 && sorted[j - 1] > sorted[j]; --j) {
      tmp = sorted[j-1];
      sorted[j-1] = sorted[j];
      sorted[j] = tmp;
    }
  }

  return sorted;
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

int * turn(void *context, int *a, int len) {
  int half, left_index, right_index;
  int *sorted_left;
  int *sorted_right; 
  int *sorted_final;
  ctx_t *ctx;

  ctx = context;

  half = len / 2;
  left_index = half / 2;

  if (len <= ctx->m) {
    return sort(a, len);
  }

  sorted_final = calloc(len, sizeof(int));
  assert(sorted_final);

  #pragma omp parallel
  {
    #pragma omp single nowait
    {
      #pragma omp task
      sorted_left = turn(ctx, a, half);
      #pragma omp task
      sorted_right = turn(ctx, a + half, len - half);
    }
    #pragma omp single nowait
    {
      #pragma omp task
      right_index = bin_search(sorted_right, sorted_left[left_index], len-half);
    }
    #pragma omp single nowait
    {
      #pragma omp task
      left_merge(sorted_final, sorted_left, sorted_right, left_index, right_index);
      #pragma omp task
      right_merge(sorted_final, sorted_left, sorted_right, left_index, 
                  half-left_index, right_index, len-half-right_index);
    }
  }

  free(sorted_left);
  free(sorted_right);

  return sorted_final;
}

void work(void *context) {
  ctx_t *ctx;

  ctx = context;

  ctx->sorted = turn(ctx, ctx->data, ctx->n);
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