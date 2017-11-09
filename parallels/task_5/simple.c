#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MASTER 0
#define FACTOR 2
#define PERIOD 50

#define UP 200
#define DOWN 201
#define LEFT 202
#define RIGHT 203


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
  int rank;
  int size;
} ctx_t;

typedef struct point_t {
  int x;
  int y;
  int iteration;
} point_t;

void stats(void *context);
void experiment(void *context);
int getseed(int rank, int size);
void push(point_t **array, int *capacity, int *size, point_t *element);
void pop(point_t **array, int *size, int *index);
int getdirection(void *context);

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
    .down = atof(argv[9]),
    .rank = rank,
    .size = size
  };

  experiment(&ctx);

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

void experiment(void *context) {
  ctx_t *ctx = context;
  int seed = getseed(ctx->rank, ctx->size);

  srand(seed);

  int pointssize = ctx->N;
  point_t *points = calloc(ctx->N, sizeof(point_t));
  assert(points);
  for (int i = 0; i < ctx->N; ++i) {
    points[i].x = rand() % ctx->l;
    points[i].y = rand() % ctx->l;
    points[i].iteration = 0;
  }

  int completedsize = 0;
  int completedcapacity = ctx->N;
  point_t *completed = calloc(ctx->N, sizeof(point_t));
  assert(completed);

  int sendupsize = 0;
  int sendupcapacity = ctx->N;
  point_t *sendup = calloc(ctx->N, sizeof(point_t));
  assert(sendup);

  int senddownsize = 0;
  int senddowncapacity = ctx->N;
  point_t *senddown = calloc(ctx->N, sizeof(point_t));
  assert(senddown);

  int sendleftsize = 0;
  int sendleftcapacity = ctx->N;
  point_t *sendleft = calloc(ctx->N, sizeof(point_t));
  assert(sendleft);

  int sendrightsize = 0;
  int sendrightcapacity = ctx->N;
  point_t *sendright = calloc(ctx->N, sizeof(point_t));
  assert(sendright);

  while (1) {
    int index = 0;
    while (index < pointssize) {
      int flag = 1;
      point_t *point = points + index;
      for (int i = 0; i < PERIOD; ++i) {
        if (point->iteration == ctx->n) {
          push(&completed, &completedcapacity, &completedsize, point);
          pop(&points, &pointssize, &index);
          flag = 0;
          break;
        } else {
          ++point->iteration;
          int direction = getdirection(ctx);
          if (direction == UP) {
            ++point->y;
            if (point->y == ctx->l) {
              point->y = 0;
              push(&sendup, &sendupcapacity, &sendupsize, point);
              pop(&points, &pointssize, &index);
              flag = 0;
              break;
            }
          } else if (direction == DOWN) {
            --point->y;
            if (point->y == 0) {
              point->y = ctx->l - 1;
              push(&senddown, &senddowncapacity, &senddownsize, point);
              pop(&points, &pointssize, &index);
              flag = 0;
              break;
            }
          } else if (direction == LEFT) {
            --point->x;
            if (point->x == 0) {
              point->x = ctx->l - 1;
              push(&sendleft, &sendleftcapacity, &sendleftsize, point);
              pop(&points, &pointssize, &index);
              flag = 0;
              break;
            }
          } else {
            ++point->x;
            if (point->x == ctx->l) {
              point->x = 0;
              push(&sendright, &sendrightcapacity, &sendrightsize, point);
              pop(&points, &pointssize, &index);
              flag = 0;
              break;
            }
          }
        }
        if (flag == 1) {
          ++index;
        }
      }
    }
    break;
  }

  if (sendup != NULL) {
    free(sendup);
  }
  if (senddown != NULL) {
    free(senddown);
  }
  if (sendleft != NULL) {
    free(sendleft);
  }
  if (sendright != NULL) {
    free(sendright);
  }
  if (completed != NULL) {
    free(completed);
  }
  if (points != NULL) {
    free(points);
  }
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
  
  if (buf != NULL) {
    free(buf);
  }
  
  return seed;
}

void push(point_t **array, int *capacity, int *size, point_t *element) {
  if (*size < *capacity) {
    (*array)[*size] = *element;
    ++(*size);
  } else {
    *array = realloc(*array, *capacity * FACTOR * sizeof(point_t));
    *capacity *= FACTOR;
    (*array)[*size] = *element;
    ++(*size);
  }
}

void pop(point_t **array, int *size, int *index) {
  (*array)[*index] = (*array)[*size - 1];
  --(*size);
}

int getdirection(void *context) {
  ctx_t *ctx = context;
  float up = ctx->up * rand();
  float down = ctx->down * rand();
  float left = ctx->left * rand();
  float right = ctx->right * rand();

  if ((up >= down) && (up >= left) && (up >= right)) {
    return UP;
  } else if ((down >= up) && (down >= left) && (down >= right)) {
    return DOWN;
  } else if ((left >= down) && (left >= up) && (left >= right)) {
    return LEFT;
  } else {
    return RIGHT;
  }
}