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

typedef struct arg {
  ctx_t *context;
  int index;
  int *data;
  int *sorted;
  int x;
  int y;
  int m;
  int n;
  int bound;
} arg;

/* ========== PROTOTYPES ========== */

void ctor(void *context);
void dtor(void *context);
void statistics(void *context);
void work(void *context);
void sort(void *context);
void *routine_sort(void *args);
int cmpfunc(const void *a, const void *b);
int bin_search(int *a, int key, int left, int right);
void merge(void *context);
void *routine_merge(void *args);
void *routine_migrate(void *args);

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
  merge(ctx);

  assert(gettimeofday(&end, NULL) == 0);

  ctx->worktime = ((end.tv_sec - start.tv_sec) * 1000000u + \
                    end.tv_usec - start.tv_usec) / 1.e6;
}

void sort(void *context) {
  int i;
  ctx_t *ctx;

  ctx = context;

  pthread_t threads[ctx->P];
  arg args[ctx->P];

  for (i = 0; i < ctx->P; ++i) {
    args[i].context = ctx;
    args[i].index = i;
  }

  for (i = 0; i < ctx->P; ++i) {
    pthread_create(&threads[i], NULL, &routine_sort, (void *)&args[i]);
  }

  for (i = 0; i < ctx->P; ++i) {
    pthread_join(threads[i], NULL);
  }
}

void *routine_sort(void *args_) {
  int index, left, right, i;
  int *buf;
  ctx_t *ctx;
  arg *args;

  args = args_;

  ctx = args->context;
  index = args->index; 

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

int bin_search(int *a, int key, int left, int right) {
  int mid;

  if (right - left <= 1) {
    if (a[left] < key) {
      return right;
    } else {
     return left;
    }
  } else {
    mid = (left + right) / 2;

    if (a[mid] < key) {
      return bin_search(a, key, mid, right);
    } else {
      return bin_search(a, key, left, mid);
    }
  }
}

void merge(void *context) {
  int num_chunks, i, median, diff, k, right_index;
  int *left_edge;
  int *right_edge;
  int *lefts;
  int *rights;
  ctx_t *ctx;

  ctx = context;

  pthread_t threads[ctx->P];
  
  if (ctx->n % ctx->m) {
    num_chunks = ctx->n / ctx->m + 1;
  } else {
    num_chunks = ctx->n / ctx->m;
  }

  left_edge = calloc(num_chunks, sizeof(int));
  assert(left_edge);

  right_edge = calloc(num_chunks, sizeof(int));
  assert(right_edge);

  for (i = 0; i < num_chunks; ++i) {
    left_edge[i] = ctx->m * i;
    right_edge[i] = ctx->m * (i + 1);

    if (right_edge[i] > ctx->n) {
      right_edge[i] = ctx->n;
    }
  }

  while (num_chunks > 1) {
    if (num_chunks <= ctx->P) {
      diff = 0;

      arg args[num_chunks];

      for (i = 0; i < num_chunks - 1; i += 2) {
        median = (left_edge[i] + right_edge[i]) / 2;
        right_index = bin_search(ctx->sorted, ctx->sorted[median], 
                                 left_edge[i + 1], right_edge[i+1]);

        args[i].data = ctx->sorted;
        args[i].x = left_edge[i];
        args[i].y = median;
        args[i].m = left_edge[i + 1];
        args[i].n = right_index;
        args[i].bound = left_edge[i];

        args[i + 1].data = ctx->sorted;
        args[i + 1].x = median;
        args[i + 1].y = right_edge[i];
        args[i + 1].m = right_index;
        args[i + 1].n = right_edge[i + 1];
        args[i + 1].bound = median + right_index - left_edge[i + 1];
      }

      for (i = 0; i < num_chunks - 1; i += 2) {
        pthread_create(&threads[i], NULL, &routine_merge, (void *)&args[i]);
        pthread_create(&threads[i + 1], NULL,
                       &routine_merge, (void *)&args[i + 1]);
        ++diff;
      }

      for (i = 0; i < num_chunks - 1; i += 2) {
        pthread_join(threads[i], NULL);
        pthread_join(threads[i + 1], NULL);
      }

      for (i = 0; i < num_chunks - 1; i += 2) {
        pthread_create(&threads[i], NULL, &routine_migrate, (void *)&args[i]);
        pthread_create(&threads[i + 1], NULL,
                       &routine_migrate, (void *)&args[i + 1]);
      }

      for (i = 0; i < num_chunks - 1; i += 2) {
        pthread_join(threads[i], NULL);
        pthread_join(threads[i + 1], NULL);
      }

      for (i = 0; i < num_chunks - 1; i += 2) {
        free(args[i].sorted);
        free(args[i + 1].sorted);
      }

      lefts = calloc(diff + 1, sizeof(int));
      rights = calloc(diff + 1, sizeof(int));
      
      for (i = 0, k = 0; i < diff; ++i, k += 2) {
        lefts[i] = left_edge[k];
        rights[i] = right_edge[k + 1];
      }

      if (num_chunks % 2) {
        lefts[diff] = left_edge[num_chunks - 1];
        rights[diff] = right_edge[num_chunks - 1];
      }

      for (i = 0; i < diff + 1; ++i) {
        left_edge[i] = lefts[i];
        right_edge[i] = rights[i];
      }

      num_chunks -= diff;

      free(lefts);
      free(rights);
    } else {
    //   diff = 0;

    //   printf("num_chunks = %d\n", num_chunks);

    //   args_merge args[ctx->P];

    //   for (i = 0; i < ctx->P - 1; i += 2) {
    //     median = (left_edge[i] + right_edge[i]) / 2;
    //     right_index = bin_search(ctx->sorted, ctx->sorted[median], 
    //                              left_edge[i + 1], right_edge[i+1]);

    //     args[i].a = ctx->sorted;
    //     args[i].x = left_edge[i];
    //     args[i].y = median;
    //     args[i].m = left_edge[i + 1];
    //     args[i].n = right_index;
    //     args[i].bound = left_edge[i];
    //     args[i + 1].a = ctx->sorted;
    //     args[i + 1].x = median;
    //     args[i + 1].y = right_edge[i];
    //     args[i + 1].m = right_index;
    //     args[i + 1].n = right_edge[i + 1];
    //     args[i + 1].bound = median + right_index - left_edge[i + 1];
    //   }

    //   for (i = 0; i < ctx->P - 1; i += 2) {
    //     pthread_create(&threads[i], NULL, &routine_merge, (void *)&args[i]);
    //     pthread_create(&threads[i + 1], NULL,
    //                    &routine_merge, (void *)&args[i + 1]);
    //     ++diff;
    //   }

    //   for (i = 0; i < ctx->P - 1; i += 2) {
    //     pthread_join(threads[i], NULL);
    //     pthread_join(threads[i + 1], NULL);
    //   }

    //   for (i = 0; i < ctx->P; ++i) {
    //     int size = args[i].y - args[i].x + args[i].n - args[i].m;
    //     for (int j = 0; j < size; ++j){
    //       printf("%d ", args[i].res[j]);
    //     }
    //     printf("\n");
    //   }

    //   for (i = 0; i < ctx->P - 1; i += 2) {
    //     pthread_create(&threads[i], NULL, &routine_migrate, (void *)&args[i]);
    //     pthread_create(&threads[i + 1], NULL,
    //                    &routine_migrate, (void *)&args[i + 1]);
    //   }

    //   for (i = 0; i < ctx->P - 1; i += 2) {
    //     pthread_join(threads[i], NULL);
    //     pthread_join(threads[i + 1], NULL);
    //   }

    //   for (i = 0; i < ctx->n; ++i) {
    //     printf("%d ", ctx->sorted[i]);
    //   }

    //   for (i = 0; i < ctx->P - 1; i += 2) {
    //     free(args[i].res);
    //     free(args[i + 1].res);
    //   }

    //   lefts = calloc(diff, sizeof(int));
    //   rights = calloc(diff, sizeof(int));
      
    //   for (i = 0, k = 0; i < diff; ++i, k += 2) {
    //     lefts[i] = left_edge[k];
    //     rights[i] = right_edge[k + 1];
    //   }

    //   for (i = 0; diff * 2 + i < num_chunks; ++i) {
    //     left_edge[i + diff] = left_edge[diff * 2 + i];
    //     right_edge[i + diff] = right_edge[diff * 2 + i];
    //   }

    //   num_chunks -= diff;

    //   printf("num_chunks new = %d\n", num_chunks);

    //   free(lefts);
    //   free(rights);
    }
  }

  free(left_edge);
  free(right_edge);
}

// int *a, int x, int y, int m, int n

void *routine_merge(void *args_) {
  int i, size, x, y, m, n;
  int *tmp;
  arg *args;

  args = args_;

  x = args->x;
  y = args->y;
  m = args->m;
  n = args->n;

  size = (n - m) + (y - x);

  tmp = calloc(size, sizeof(int));
  assert(tmp);

  i = 0;

  while(1) {
    if (x == y) {
      while (m != n) {
        tmp[i] = args->data[m];
        ++i;
        ++m;
      }

      break;
    }

    if (m == n) {
      while (x != y) {
        tmp[i] = args->data[x];
        ++i;
        ++x;
      }

      break;
    }

    if (args->data[x] < args->data[m]) {
      tmp[i] = args->data[x];
      ++x;
    } else {
      tmp[i] = args->data[m];
      ++m;
    }

    ++i;
  }

  args->sorted = calloc(size, sizeof(int));

  for (i = 0; i < size; ++i) {
    args->sorted[i] = tmp[i];
  }

  free(tmp);
}

void *routine_migrate(void *args_) {
  int i, size;
  arg *args;

  args = args_;

  size = args->y - args->x + args->n - args->m;

  for (i = 0; i < size; ++i) {
    args->data[args->bound + i] = args->sorted[i];
  }
}












