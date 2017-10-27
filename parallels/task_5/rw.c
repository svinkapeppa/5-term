#include <assert.h>
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

#define MASTER 0
#define UP 8
#define DOWN 2
#define LEFT 4
#define RIGHT 6

typedef struct walker_t {
  int x;
  int y;
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

  for (int i = 0; i < ctx->n; ++i) {
    for (int j = 0; j < *workingnumber; ++j) {
      walk(ctx, rank, size, j, working, workingnumber,
           up, upnumber, down, downnumber,
           left, leftnumber, right, rightnumber);
    }
  }

  free(working);
  free(up);
  free(down);
  free(left);
  free(right);
  free(upnumber);
  free(downnumber);
  free(leftnumber);
  free(rightnumber);
  free(workingnumber);
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
        // 1. Delete from working
        // 2. Decrease workingnumber
        // 3. Add to up
        // 4. Increase upnumber
        walker_t *upbuf = calloc(*upnumber, sizeof(walker_t));
        assert(upnumber);
        for (int i = 0; i < *upnumber - 1; ++i) {
          upbuf[i] = up[i];
        }

        (*upnumber)++;
        up = calloc(*upnumber, sizeof(walker_t));
        for (int i = 0; i < *upnumber - 1; ++i) {
          up[i] = upbuf[i];
        }
        up[*upnumber - 1] = working[index];

        walker_t *buf = calloc(*workingnumber - 1, sizeof(walker_t));
        assert(buf);
        for (int i = 0; i < index; ++i) {
          buf[i] = working[i];
        }
        for (int i = index + 1; i < *workingnumber; ++i) {
          buf[i - 1] = working[i];
        }
        (*workingnumber)--;
        working = realloc(working, *workingnumber * sizeof(walker_t));
        for (int i = 0; i < *workingnumber; ++i) {
          working[i] = buf[i];
        }

        free(upbuf);
        free(buf);
      }
      break;
    case DOWN:
      working[index].y--;
      break;
    case LEFT:
      working[index].x--;
      break;
    case RIGHT:
      working[index].x++;
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
    working[i] = walker;
  }  
}