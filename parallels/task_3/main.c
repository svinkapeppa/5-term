#include <stdio.h>
#include <stdlib.h>

#include "library.h"
#include "runner.h"

int main(int argc, char **argv) {
  int n, m, P;

  n = atoi(argv[1]);
  m = atoi(argv[2]);
  P = atoi(argv[3]);

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