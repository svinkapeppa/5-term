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

int getdirection(void *context);
int getdestination(void *context, int direction);
void push(point_t **array, int *size, int *capacity, point_t *element);
void pop(point_t **array, int *size, int index);
void finish(void *context, int *completedsize, int *done, struct timeval start);
int getseed(int rank, int size);
void experiment(void *context);
void stats(void *context, int *distribution, struct timeval start);
void communicatesizes(int left, int right, int up, int down,
                      int *sendleftsize, int *sendrightsize,
                      int *sendupsize, int *senddownsize,
                      int *receiveleftsize, int *receiverightsize,
                      int *receiveupsize, int *receivedownsize);
void communicatedata(int left, int right, int up, int down,
                     int receiveleftsize, int receiverightsize,
                     int receiveupsize, int receivedownsize,
                     int *sendleftsize, int *sendrightsize,
                     int *sendupsize, int *senddownsize,
                     point_t *sendleft, point_t *sendright,
                     point_t *sendup, point_t *senddown,
                     point_t *receiveleft, point_t *receiveright,
                     point_t *receiveup, point_t *receivedown);