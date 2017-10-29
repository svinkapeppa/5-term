#include <assert.h>
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

#define MASTER 0
#define UP 8
#define DOWN 2
#define LEFT 4
#define RIGHT 6
#define FORWARD 0
#define BACKWARD 1

typedef struct walker_t {
  int x;
  int y;
  int status;
} walker_t;

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
} ctx_t;

int max(double up, double down, double left, double right);
int step(void *context);
void experiment(void *context, int rank, int size);
void walk(void *context, int rank, int size, int index, walker_t *working, 
          int *workingnumber, walker_t *up, int *upnumber, walker_t *down, int *downnumber,
          walker_t *left, int *leftnumber, walker_t *right, int *rightnumber);
void init(void *context, int rank, int size, walker_t *incoming);
void communicate(void *context, int rank, int size, walker_t *up, int *upnumber,
                 int direction);

int main(int argc, char **argv)
{
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
    .down = atof(argv[9])
  };

  experiment(&ctx, rank, size);
 
  MPI_Finalize();
  return 0;
}

int max(double up, double down, double left, double right)
{
  if (up >= down && up >= left && up >= right) {
    return UP; 
  } else if (down >= up, down >= left, down >= right) {
    return DOWN;
  } else if (left >= up, left >= down, left >= right) {
    return LEFT;
  } else {
    return RIGHT;
  }
}

int step(void *context)
{
  ctx_t *ctx = context;

  double bidderup = ctx->up * rand();
  double bidderdown = ctx->down * rand();
  double bidderleft = ctx->left * rand();
  double bidderright = ctx->right * rand();

  return max(bidderup, bidderdown, bidderleft, bidderright);
}

void experiment(void *context, int rank, int size)
{
  ctx_t *ctx = context;
  walker_t *up = calloc(1, sizeof(walker_t));
  assert(up);
  walker_t *down = calloc(1, sizeof(walker_t));
  assert(down);
  walker_t *left = calloc(1, sizeof(walker_t));
  assert(left);
  walker_t *right = calloc(1, sizeof(walker_t));
  assert(right);
  walker_t *working = calloc(ctx->N, sizeof(walker_t));
  assert(working);
  int *upnumber = calloc(1, sizeof(int));
  assert(upnumber);
  *upnumber = 0;
  int *downnumber = calloc(1, sizeof(int));
  assert(downnumber);
  *downnumber = 0;
  int *leftnumber = calloc(1, sizeof(int));
  assert(leftnumber);
  *leftnumber = 0;
  int *rightnumber = calloc(1, sizeof(int));
  assert(rightnumber);
  *rightnumber = 0;
  int *workingnumber = calloc(1, sizeof(int));
  assert(workingnumber);
  *workingnumber = ctx->N;

  init(context, rank, size, working);

  for (int i = 0; i < 1; ++i) {
    int temp = 0;
    int k = 0;
    for (int j = 0; j < *workingnumber; ++j) {
      walk(ctx, rank, size, j, working, workingnumber,
           up, upnumber, down, downnumber,
           left, leftnumber, right, rightnumber);
      if (working[j].status == 1) {
        temp++;
      }
    }
    walker_t *buf = calloc(*workingnumber - temp, sizeof(walker_t));
    assert(buf);
    for (int j = 0; j < *workingnumber; ++j) {
      if (working[j].status == 0) {
        buf[k] = working[j];
        k++;
      } else {
        working[j].status = 0;
      }
    }
    working = calloc(*workingnumber - temp, sizeof(walker_t));
    assert(working);
    for (int j = 0; j < (*workingnumber - temp); ++j) {
      working[j] = buf[j];
    }
    if (buf) {
      free(buf);
    }
    if (rank % 2 == 0) {
      // Create function to do this
      communicate(context, rank, size, up, upnumber, FORWARD);
      // Need to do 3 more sends and 4 recieves
    } else {
      communicate(context, rank, size, up, upnumber, BACKWARD);
      // Backwards
    }
  }

  if (working) {
    free(working);
  }
  if (up) {
    free(up);
  }
  if (down) {
    free(down);
  }
  if (left) {
    free(left);
  }
  if (right) {
    free(right); 
  }
  if (upnumber) {
    free(upnumber);
  }
  if (downnumber) {
    free(downnumber);
  }
  if (leftnumber) {
    free(leftnumber);
  }
  if (rightnumber) {
    free(rightnumber); 
  }
  if (workingnumber) {
    free(workingnumber);
  }
}

void walk(void *context, int rank, int size, int index, walker_t *working, 
          int *workingnumber, walker_t *up, int *upnumber, walker_t *down, int *downnumber,
          walker_t *left, int *leftnumber, walker_t *right, int *rightnumber)
{
  ctx_t *ctx = context;
  int direction = step(context);
  switch (direction)
  {
    case UP:
      working[index].y++;
      if (working[index].y % ctx->l == 0) {
        walker_t *upbuf = calloc(*upnumber, sizeof(walker_t));
        assert(upbuf);
        for (int i = 0; i < *upnumber; ++i) {
          upbuf[i] = up[i];
        }

        (*upnumber)++;
        up = calloc(*upnumber, sizeof(walker_t));
        for (int i = 0; i < *upnumber - 1; ++i) {
          up[i] = upbuf[i];
        }
        up[*upnumber - 1] = working[index];

        working[index].status = 1;
        if (upbuf) {
          free(upbuf); 
        }
      }
      break;
    case DOWN:
      working[index].y--;
      if (working[index].y % ctx->l == 0) {
        walker_t *downbuf = calloc(*downnumber, sizeof(walker_t));
        assert(downbuf);
        for (int i = 0; i < *downnumber; ++i) {
          downbuf[i] = down[i];
        }

        (*downnumber)++;
        down = calloc(*downnumber, sizeof(walker_t));
        for (int i = 0; i < *downnumber - 1; ++i) {
          down[i] = downbuf[i];
        }
        down[*downnumber - 1] = working[index];

        working[index].status = 1;
        if (downbuf) {
          free(downbuf);
        }
      }
      break;
    case LEFT:
      working[index].x--;
      if (working[index].x % ctx->l == 0) {
        walker_t *leftbuf = calloc(*leftnumber, sizeof(walker_t));
        assert(leftbuf);
        for (int i = 0; i < *leftnumber; ++i) {
          leftbuf[i] = left[i];
        }

        (*leftnumber)++;
        left = calloc(*leftnumber, sizeof(walker_t));
        for (int i = 0; i < *leftnumber - 1; ++i) {
          left[i] = leftbuf[i];
        }
        left[*leftnumber - 1] = working[index];

        working[index].status = 1;
        if (leftbuf) {
          free(leftbuf);
        }
      }
      break;
    case RIGHT:
      working[index].x++;
      if (working[index].x % ctx->l == 0) {
        walker_t *rightbuf = calloc(*rightnumber, sizeof(walker_t));
        assert(rightbuf);
        for (int i = 0; i < *rightnumber; ++i) {
          rightbuf[i] = right[i];
        }

        (*rightnumber)++;
        right = calloc(*rightnumber, sizeof(walker_t));
        for (int i = 0; i < *rightnumber - 1; ++i) {
          right[i] = rightbuf[i];
        }
        right[*rightnumber - 1] = working[index];

        working[index].status = 1;
        if (rightbuf) {
          free(rightbuf);
        }
      }
      break;
    default:
      fprintf(stderr, "WRONG DIRECTION! ABORT!\n");
  }
}

void init(void *context, int rank, int size, walker_t *working)
{
  ctx_t *ctx = context;
  walker_t walker;

  srand(time(NULL) + rank);

  int node = rank % size;
  int row = node / ctx->b;
  int column = node % ctx-> a;

  for (int i = 0; i < ctx->N; ++i) {
    walker.x = row * ctx->l + rand() % ctx->l;
    walker.y = column * ctx->l + rand() % ctx->l;
    walker.status = 0;
    working[i] = walker;
  }  
}

void communicate(void *context, int rank, int size, walker_t *up, int *upnumber,
                 int direction)
{
  ctx_t *ctx = context;
  MPI_Status status;
  int length;
  int node;
  walker_t *uprecv;
  switch(direction)
  {
    case FORWARD:
      node = (rank + ctx->a) % (ctx->a * ctx->b);
      MPI_Send((void *)up, *upnumber * sizeof(walker_t), MPI_BYTE,
               node, 0, MPI_COMM_WORLD);
      MPI_Probe(node, 0, MPI_COMM_WORLD, &status);
      MPI_Get_count(&status, MPI_BYTE, &length);
      uprecv = calloc(length / sizeof(walker_t), sizeof(walker_t));
      assert(uprecv);
      MPI_Recv((void *)uprecv, length, MPI_BYTE, node,
               0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      break;
    case BACKWARD:
      node = (rank + ctx->a) % (ctx->a * ctx->b);
      MPI_Probe(node, 0, MPI_COMM_WORLD, &status);
      MPI_Get_count(&status, MPI_BYTE, &length);
      uprecv = calloc(length / sizeof(walker_t), sizeof(walker_t));
      assert(uprecv);
      MPI_Recv((void *)uprecv, length, MPI_BYTE, node,
               0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Send((void *)up, *upnumber * sizeof(walker_t), MPI_BYTE,
               node, 0, MPI_COMM_WORLD);
      break;
    default:
      fprintf(stderr, "WRONG COMMUNICATION SEQUENCE\n");
  } 
}