#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MASTER 0
#define FACTOR 2
#define PERIOD 200

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
void pop(point_t **array, int *size, int index);
int getdirection(void *context);
void communicate(void *context, int *sendupsize, int *senddownsize, int *sendleftsize,
                 int *sendrightsize, int *recieveupsize, int *recievedownsize,
                 int *recieveleftsize, int *recieverightsize);
int getdestination(void *context, int direction);
void update(void *context, point_t *sendup, point_t *senddown,
            point_t *sendleft, point_t *sendright, int *sendupsize,
            int *senddownsize, int *sendleftsize, int *sendrightsize,
            int *recieveupsize, int *recievedownsize, int *recieveleftsize,
            int *recieverightsize, point_t *points, int *pointscapacity,
            int *pointssize);
void finish(void *context, int *completedsize, int *done);
void work(void *context, point_t *points, int *pointssize,
          point_t *completed, int *completedcapacity, int *completedsize,
          point_t *sendup, int *sendupcapacity, int *sendupsize,
          point_t *senddown, int *senddowncapacity, int *senddownsize,
          point_t *sendleft, int *sendleftcapacity, int *sendleftsize,
          point_t *sendright, int *sendrightcapacity, int *sendrightsize);
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
  int pointscapacity = ctx->N;
  point_t *points = malloc(ctx->N * sizeof(point_t));
  for (int i = 0; i < ctx->N; ++i) {
    points[i].x = rand() % ctx->l;
    points[i].y = rand() % ctx->l;
    points[i].iteration = 0;
  }

  int recieveupsize = 0;
  int recievedownsize = 0;
  int recieveleftsize = 0;
  int recieverightsize = 0;

  int completedsize = 0;
  int completedcapacity = ctx->N;
  point_t *completed = malloc(ctx->N * sizeof(point_t));

  int sendupsize = 0;
  int sendupcapacity = ctx->N;
  point_t *sendup = malloc(ctx->N * sizeof(point_t));

  int senddownsize = 0;
  int senddowncapacity = ctx->N;
  point_t *senddown = malloc(ctx->N * sizeof(point_t));

  int sendleftsize = 0;
  int sendleftcapacity = ctx->N;
  point_t *sendleft = malloc(ctx->N * sizeof(point_t));

  int sendrightsize = 0;
  int sendrightcapacity = ctx->N;
  point_t *sendright = malloc(ctx->N * sizeof(point_t));

  int done = 0;
  while (1) {
    work(ctx, points, &pointssize,
         completed, &completedcapacity, &completedsize,
         sendup, &sendupcapacity, &sendupsize,
         senddown, &senddowncapacity, &senddownsize,
         sendleft, &sendleftcapacity, &sendleftsize,
         sendright, &sendrightcapacity, &sendrightsize);
    communicate(ctx, &sendupsize, &senddownsize, &sendleftsize,
                &sendrightsize, &recieveupsize, &recievedownsize,
                &recieveleftsize, &recieverightsize);
    update(ctx, sendup, senddown, sendleft, sendright,
           &sendupsize, &senddownsize, &sendleftsize,
           &sendrightsize, &recieveupsize, &recievedownsize,
           &recieveleftsize, &recieverightsize, points,
           &pointscapacity, &pointssize);
    if (done == 1) {
      break;
    }
    MPI_Barrier(MPI_COMM_WORLD);
    break;
  }

  free(sendup);
  free(senddown);
  free(sendleft);
  free(sendright);
  free(completed);
  free(points);
}

int getseed(int rank, int size) {
  int *buf = NULL;
  int seed;

  if (rank == MASTER) {
    srand(time(NULL));
    buf = malloc(size * sizeof(int));
    for (int i = 0; i < size; ++i) {
      buf[i] = rand();
    }
  }

  MPI_Scatter(buf, 1, MPI_INT, &seed, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
  
  free(buf);
  
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

void pop(point_t **array, int *size, int index) {
  (*array)[index] = (*array)[(*size) - 1];
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

void communicate(void *context, int *sendupsize, int *senddownsize, int *sendleftsize,
                 int *sendrightsize, int *recieveupsize, int *recievedownsize,
                 int *recieveleftsize, int *recieverightsize) {
  ctx_t *ctx = context;
  int up = getdestination(ctx, UP);
  int down = getdestination(ctx, DOWN);
  int left = getdestination(ctx, LEFT);
  int right = getdestination(ctx, RIGHT);

  MPI_Request *sizes = malloc(8 * sizeof(MPI_Request));

  MPI_Isend(sendupsize, 1, MPI_INT, up, 0, MPI_COMM_WORLD, sizes + 0);
  MPI_Isend(senddownsize, 1, MPI_INT, down, 1, MPI_COMM_WORLD, sizes + 1);
  MPI_Isend(sendleftsize, 1, MPI_INT, left, 2, MPI_COMM_WORLD, sizes + 2);
  MPI_Isend(sendrightsize, 1, MPI_INT, right, 3 , MPI_COMM_WORLD, sizes + 3);

  MPI_Irecv(recieveupsize, 1, MPI_INT, up, 1, MPI_COMM_WORLD, sizes + 4);
  MPI_Irecv(recievedownsize, 1, MPI_INT, down, 0, MPI_COMM_WORLD, sizes + 5);
  MPI_Irecv(recieveleftsize, 1, MPI_INT, left, 3, MPI_COMM_WORLD, sizes + 6);
  MPI_Irecv(recieverightsize, 1, MPI_INT, right, 2, MPI_COMM_WORLD, sizes + 7);

  MPI_Waitall(8, sizes, MPI_STATUS_IGNORE);
  free(sizes);
}

int getdestination(void *context, int direction) {
  ctx_t *ctx = context;
  int row = ctx->rank / ctx->a;
  int column = ctx->rank % ctx->a;
  switch (direction) {
    case UP:
      ++row;
      if (row >= ctx->b) {
        row = 0;
      }
      break;
    case DOWN:
      --row;
      if (row < 0) {
        row = ctx->b - 1;
      }
      break;
    case LEFT:
      --column;
      if (column < 0) {
        column = ctx->a - 1;
      }
      break;
    case RIGHT:
      ++column;
      if (column >= ctx->a) {
        column = 0;
      }
      break;
    default:
      fprintf(stderr, "WRONG DIRECTION! ABORT!\n");
  }
  return row * ctx->a + column;
}

void update(void *context, point_t *sendup, point_t *senddown,
            point_t *sendleft, point_t *sendright, int *sendupsize,
            int *senddownsize, int *sendleftsize, int *sendrightsize,
            int *recieveupsize, int *recievedownsize, int *recieveleftsize,
            int *recieverightsize, point_t *points, int *pointscapacity,
            int *pointssize) {
  ctx_t *ctx = context;
  point_t *recieveup = malloc(*recieveupsize * sizeof(point_t));
  point_t *recievedown = malloc(*recievedownsize * sizeof(point_t));
  point_t *recieveleft = malloc(*recieveleftsize * sizeof(point_t));
  point_t *recieveright = malloc(*recieverightsize * sizeof(point_t));
  
  int up = getdestination(ctx, UP);
  int down = getdestination(ctx, DOWN);
  int left = getdestination(ctx, LEFT);
  int right = getdestination(ctx, RIGHT);

  MPI_Request *data= malloc(8 * sizeof(MPI_Request));

  MPI_Issend(sendup, *sendupsize * sizeof(point_t), MPI_BYTE, up, 0, MPI_COMM_WORLD, data + 0);
  MPI_Issend(senddown, *senddownsize * sizeof(point_t), MPI_BYTE, down, 1, MPI_COMM_WORLD, data + 1);
  MPI_Issend(sendleft, *sendleftsize * sizeof(point_t), MPI_BYTE, left, 2, MPI_COMM_WORLD, data + 2);
  MPI_Issend(sendright, *sendrightsize * sizeof(point_t), MPI_BYTE, right, 3 , MPI_COMM_WORLD, data + 3);

  MPI_Irecv(recieveup, *recieveupsize * sizeof(point_t), MPI_BYTE, up, 1, MPI_COMM_WORLD, data + 4);
  MPI_Irecv(recievedown, *recievedownsize * sizeof(point_t), MPI_BYTE, down, 0, MPI_COMM_WORLD, data + 5);
  MPI_Irecv(recieveleft, *recieveleftsize * sizeof(point_t), MPI_BYTE, left, 3, MPI_COMM_WORLD, data + 6);
  MPI_Irecv(recieveright, *recieverightsize * sizeof(point_t), MPI_BYTE, right, 2, MPI_COMM_WORLD, data + 7);

  MPI_Waitall(8, data, MPI_STATUS_IGNORE);

  for (int i = 0; i < *recieveupsize; ++i) {
    push(&points, pointscapacity, pointssize, recieveup + i);
  }
  for (int i = 0; i < *recievedownsize; ++i) {
    push(&points, pointscapacity, pointssize, recievedown + i);
  }
  for (int i = 0; i < *recieveleftsize; ++i) {
    push(&points, pointscapacity, pointssize, recieveleft + i);
  }
  for (int i = 0; i < *recieverightsize; ++i) {
    push(&points, pointscapacity, pointssize, recieveright + i);
  }

  *sendupsize = 0;
  *senddownsize = 0;
  *sendleftsize = 0;
  *sendrightsize = 0;

  free(recieveup);
  free(recievedown);
  free(recieveleft);
  free(recieveright);
  free(data);
}

void finish(void *context, int *completedsize, int *done) {
  ctx_t *ctx = context;
  int alreadycompleted = 0;
  MPI_Reduce(completedsize, &alreadycompleted, 1, MPI_INT, MPI_SUM, MASTER, MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);
  if ((ctx->rank == MASTER) && (alreadycompleted == ctx->size * ctx->N)) {
    *done = 1;
  }
  MPI_Bcast(done  , 1, MPI_INT, MASTER, MPI_COMM_WORLD);
  if (*done == 1) {
    int *distribution = malloc(ctx->size * sizeof(int));
    MPI_Gather(completedsize, 1, MPI_INT, distribution, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    if (ctx->rank == MASTER) {
      stats(ctx);
      for (int i = 0; i < ctx->size; ++i) {
        printf("rank = %d points = %d\n", i, distribution[i]);
      }
    }
    free(distribution);
  }
}

void work(void *context, point_t *points, int *pointssize,
          point_t *completed, int *completedcapacity, int *completedsize,
          point_t *sendup, int *sendupcapacity, int *sendupsize,
          point_t *senddown, int *senddowncapacity, int *senddownsize,
          point_t *sendleft, int *sendleftcapacity, int *sendleftsize,
          point_t *sendright, int *sendrightcapacity, int *sendrightsize) {
  ctx_t *ctx = context;
  int index = 0;
  while (index < *pointssize) {
    int flag = 1;
    point_t *point = points + index;
    for (int i = 0; i < PERIOD; ++i) {
      if (point->iteration == ctx->n) {
        flag = 0;
        push(&completed, completedcapacity, completedsize, point);
        pop(&points, pointssize, index);
        break;
      }
      ++point->iteration;
      int direction = getdirection(ctx);
      if (direction == UP) {
        ++point->y;
      }
      if (direction == DOWN) {
        --point->y;
      }
      if (direction == LEFT) {
        --point->x;
      }
      if (direction == RIGHT) {
        ++point->x;
      }
      if (point->y >= ctx->l) {
          flag = 0;
          point->y = 0;
          push(&sendup, sendupcapacity, sendupsize, point);
          pop(&points, pointssize, index);
          break;
      }
      if (point->y < 0) {
        flag = 0;
        point->y = ctx->l - 1;
        push(&senddown, senddowncapacity, senddownsize, point);
        pop(&points, pointssize, index);
        break;
      }
      if (point->x >= ctx->l) {
        flag = 0;
        point->x = 0;
        push(&sendright, sendrightcapacity, sendrightsize, point);
        pop(&points, pointssize, index);
        break;
      }
      if (point->x < 0) {
        flag = 0;
        point->x = ctx->l - 1;
        push(&sendleft, sendleftcapacity, sendleftsize, point);
        pop(&points, pointssize, index);
        break;
      }
    }
    if (flag == 1) {
      ++index;
    }
  } 
}