#define MASTER 0

typedef struct ctx_t {
  int l;
  int a;
  int b;
  int N;
  int rank;
  int size;
} ctx_t;

void experiment(void *context);
int getseed(int rank, int size);
int *getdata(void *context, int *seed);
void stats(void *context, double workingtime);