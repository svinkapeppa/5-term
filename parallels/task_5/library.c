#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <mpi.h>

#include "library.h"

int getdirection(void *context) {
  ctx_t *ctx = context;
  double up = ctx->up * rand();
  double down = ctx->down * rand();
  double left = ctx->left * rand();
  double right = ctx->right * rand();

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

void push(point_t **array, int *size, int *capacity, point_t *element) {
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

void finish(void *context, int *completedsize, int *done,
            struct timeval start) {
  ctx_t *ctx = context;

  int alreadycompleted = 0;
  MPI_Reduce(completedsize, &alreadycompleted, 1, MPI_INT, MPI_SUM,
             0, MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);

  if ((ctx->rank == 0) && (alreadycompleted == ctx->size * ctx->N)) {
    *done = 1;
  }
  MPI_Bcast(done, 1, MPI_INT, 0, MPI_COMM_WORLD);
  if (*done == 1) {
    int *distribution = calloc(ctx->size, sizeof(int));
    assert(distribution);

    MPI_Gather(completedsize, 1, MPI_INT, distribution, 1, MPI_INT,
               0, MPI_COMM_WORLD);
    
    if (ctx->rank == 0) {
      stats(ctx, distribution, start);
    }
    
    free(distribution);
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
  
  free(buf);
  
  return seed;
}

void experiment(void *context) {
  ctx_t *ctx = context;
  
  int sendleftsize, sendrightsize, sendupsize, senddownsize,
      completedsize, pointssize, sendleftcapacity, sendrightcapacity,
      sendupcapacity, senddowncapacity, completedcapacity,
      pointscapacity, receiveleftsize, receiverightsize,
      receiveupsize, receivedownsize, left, right, up, down,
      index, flag, seed, direction, i;
  
  seed = getseed(ctx->rank, ctx->size);
  srand(seed);

  sendleftsize = sendrightsize = sendupsize = senddownsize =\
                 completedsize = receiveleftsize =\
                 receiverightsize = receiveupsize =\
                 receivedownsize = 0;
  pointssize = sendleftcapacity = sendrightcapacity =\
               sendupcapacity = senddowncapacity =\
               completedcapacity = pointscapacity = ctx->N;
  
  left = getdestination(ctx, LEFT);
  right = getdestination(ctx, RIGHT);
  up = getdestination(ctx, UP);
  down = getdestination(ctx, DOWN);

  point_t* sendleft = calloc(sendleftcapacity, sizeof(point_t));
  point_t* sendright = calloc(sendrightcapacity, sizeof(point_t));
  point_t* sendup = calloc(sendupcapacity, sizeof(point_t));
  point_t* senddown = calloc(senddowncapacity, sizeof(point_t));
  point_t* completed = calloc(completedcapacity, sizeof(point_t));
  point_t* points = calloc(pointscapacity, sizeof(point_t));

  assert(sendleft);
  assert(sendright);
  assert(sendup);
  assert(senddown);
  assert(completed);
  assert(points);

  for (i = 0; i < ctx->N; ++i) {
    points[i].x = rand() % ctx->l;
    points[i].y = rand() % ctx->l;
    points[i].iteration = 0;
  }
  
  struct timeval start;
  assert(gettimeofday(&start, NULL) == 0);
  
  while (1) {
    index = 0;
    while (index < pointssize) {
      point_t* point = points + index;
      flag = 1;
      for (i = 0; i < PERIOD; ++i) {
        if (point->iteration == ctx->n) {
          push(&completed, &completedsize, &completedcapacity, point);
          pop(&points, &pointssize, index);
          flag = 0;
          break;
        }
        point->iteration += 1;
        direction = getdirection(ctx);
        if (direction == LEFT) {
          --point->x;
        } else if (direction == RIGHT) {
          ++point->x;
        } else if (direction == UP) {
          ++point->y;
        } else {
          --point->y;
        }
        if (point->x < 0) {
          point->x = ctx->l - 1;
          push(&sendleft, &sendleftsize, &sendleftcapacity, point);
          pop(&points, &pointssize, index);
          flag = 0;
          break;
        }
        if (point->x >= ctx->l) {
          point->x = 0;
          push(&sendright, &sendrightsize, &sendrightcapacity, point);
          pop(&points, &pointssize, index);
          flag = 0;
          break;
        }
        if (point->y < 0) {
          point->y = ctx->l - 1;
          push(&senddown, &senddownsize, &senddowncapacity, point);
          pop(&points, &pointssize, index);
          flag = 0;
          break;
        }
        if (point->y >= ctx->l) {
          point->y = 0;
          push(&sendup, &sendupsize, &sendupcapacity, point);
          pop(&points, &pointssize, index);
          flag = 0;
          break;
        }
      }
      if (flag == 1) {
        index += 1;
      }
    }
    
    communicatesizes(left, right, up, down,
                     &sendleftsize, &sendrightsize,
                     &sendupsize, &senddownsize,
                     &receiveleftsize, &receiverightsize,
                     &receiveupsize, &receivedownsize);

    point_t* receiveleft = calloc(receiveleftsize, sizeof(point_t));
    point_t* receiveright = calloc(receiverightsize, sizeof(point_t));
    point_t* receiveup = calloc(receiveupsize, sizeof(point_t));
    point_t* receivedown = calloc(receivedownsize, sizeof(point_t));
    assert(receiveleft);
    assert(receiveright);
    assert(receiveup);
    assert(receivedown);

    communicatedata(left, right, up, down,
                    receiveleftsize, receiverightsize,
                    receiveupsize, receivedownsize,
                    &sendleftsize, &sendrightsize,
                    &sendupsize, &senddownsize,
                    sendleft, sendright,
                    sendup, senddown,
                    receiveleft, receiveright,
                    receiveup, receivedown);

    for (i = 0; i < receiveleftsize; ++i) {
        push(&points, &pointssize, &pointscapacity, receiveleft + i);
    }
    for (i = 0; i < receiverightsize; ++i) {
        push(&points, &pointssize, &pointscapacity, receiveright + i);
    }
    for (i = 0; i < receiveupsize; ++i) {
        push(&points, &pointssize, &pointscapacity, receiveup + i);
    }
    for (i = 0; i < receivedownsize; ++i) {
        push(&points, &pointssize, &pointscapacity, receivedown + i);
    }      
    
    free(receiveleft);
    free(receiveright);
    free(receiveup);
    free(receivedown);
    
    int done = 0;
    finish(ctx, &completedsize, &done, start);
    if (done == 1) {
        break;
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
  }
  
  free(sendleft);
  free(sendright);
  free(sendup);
  free(senddown);
  free(points);
  free(completed);
}

void stats(void *context, int *distribution, struct timeval start) {
  ctx_t *ctx = context;
  FILE *fd;
  int check = 0;

  fd = fopen("stats.txt", "w+");

  if (fd == NULL) {
    fprintf(stderr, "Unable to print stats\n");
    exit(128);
  } else {
    struct timeval end;
    assert(gettimeofday(&end, NULL) == 0);
    double worktime = ((end.tv_sec - start.tv_sec) * 1000000u +\
                       end.tv_usec - start.tv_usec) / 1.e6;

    fprintf(fd, "%d %d %d %d %d %.2f %.2f %.2f %.2f %.4f\n",
            ctx->l, ctx->a, ctx->b, ctx->n, ctx->N,
            ctx->left, ctx->right, ctx->up, ctx->down, worktime);
    for (int i = 0; i < ctx->size; ++i) {
      fprintf(fd, "area = %d : number of points = %d\n",
              i, distribution[i]);
      check += distribution[i];
    }
    if (check == ctx->N * ctx->size) {
      fprintf(fd, "Total number of point is %d. Everything is correct.\n",
              check);
    }
  }

  fclose(fd);
}

void communicatesizes(int left, int right, int up, int down,
                      int *sendleftsize, int *sendrightsize,
                      int *sendupsize, int *senddownsize,
                      int *receiveleftsize, int *receiverightsize,
                      int *receiveupsize, int *receivedownsize) {
  MPI_Request *sizes = calloc(8, sizeof(MPI_Request));
  assert(sizes);

  MPI_Isend(sendleftsize, 1, MPI_INT, left, 0, MPI_COMM_WORLD,
            sizes + 0);
  MPI_Isend(sendrightsize, 1, MPI_INT, right, 1, MPI_COMM_WORLD,
            sizes + 1);
  MPI_Isend(sendupsize, 1, MPI_INT, up, 2, MPI_COMM_WORLD,
            sizes + 2);
  MPI_Isend(senddownsize, 1, MPI_INT, down, 3, MPI_COMM_WORLD,
            sizes + 3);

  MPI_Irecv(receiveleftsize, 1, MPI_INT, left, 1, MPI_COMM_WORLD,
            sizes + 4);
  MPI_Irecv(receiverightsize, 1, MPI_INT, right, 0, MPI_COMM_WORLD,
            sizes + 5);
  MPI_Irecv(receiveupsize, 1, MPI_INT, up, 3, MPI_COMM_WORLD,
            sizes + 6);
  MPI_Irecv(receivedownsize, 1, MPI_INT, down, 2, MPI_COMM_WORLD,
            sizes + 7);

  MPI_Waitall(8, sizes, MPI_STATUS_IGNORE);
  free(sizes);
}

void communicatedata(int left, int right, int up, int down,
                     int receiveleftsize, int receiverightsize,
                     int receiveupsize, int receivedownsize,
                     int *sendleftsize, int *sendrightsize,
                     int *sendupsize, int *senddownsize,
                     point_t *sendleft, point_t *sendright,
                     point_t *sendup, point_t *senddown,
                     point_t *receiveleft, point_t *receiveright,
                     point_t *receiveup, point_t *receivedown) {
  MPI_Request *data = calloc(8, sizeof(MPI_Request));
  assert(data);

  MPI_Isend(sendleft, *sendleftsize * sizeof(point_t), MPI_BYTE,
            left, 0, MPI_COMM_WORLD, data + 0);
  MPI_Isend(sendright, *sendrightsize * sizeof(point_t), MPI_BYTE,
            right, 1, MPI_COMM_WORLD, data + 1);
  MPI_Isend(sendup, *sendupsize * sizeof(point_t), MPI_BYTE,
            up, 2, MPI_COMM_WORLD, data + 2);
  MPI_Isend(senddown, *senddownsize * sizeof(point_t), MPI_BYTE,
            down, 3, MPI_COMM_WORLD, data + 3);

  MPI_Irecv(receiveleft, sizeof(point_t) * receiveleftsize, MPI_BYTE,
            left, 1, MPI_COMM_WORLD, data + 4);
  MPI_Irecv(receiveright, sizeof(point_t) * receiverightsize, MPI_BYTE,
            right, 0, MPI_COMM_WORLD, data + 5);
  MPI_Irecv(receiveup, sizeof(point_t) * receiveupsize, MPI_BYTE,
            up, 3, MPI_COMM_WORLD, data + 6);
  MPI_Irecv(receivedown, sizeof(point_t) * receivedownsize, MPI_BYTE,
            down, 2, MPI_COMM_WORLD, data + 7);
  
  MPI_Waitall(8, data, MPI_STATUS_IGNORE);

  *sendleftsize = *sendrightsize = *sendupsize = *senddownsize = 0;

  free(data);
}