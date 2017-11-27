#include <mpi.h>
#include <stdlib.h>

#include "library.h"

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  ctx_t ctx = {
    .l = atoi(argv[1]),
    .a = atoi(argv[2]),
    .b = atoi(argv[3]),
    .N = atoi(argv[4]),
    .rank = rank,
    .size = size
  };
  
  double start = MPI_Wtime();

  experiment(&ctx);

  double finish = MPI_Wtime();

  stats(&ctx, finish - start);

  MPI_Finalize();

  return 0;
}