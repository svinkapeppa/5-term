#include <assert.h>
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

#define MASTER 0

void walk(int rank, int size, int *seed)
{
  int dst = rank % size;
  int v;
  if (rank == MASTER) {
    v = rand_r(&seed[rank]);
    printf("MASTER = %d\n", v);
  } else {
    v = rand_r(&seed[rank]) % 100;
    printf("rank %d: slave = %d\n", dst, v);
  }
}

int main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  srand(time(NULL));
  int *seed = calloc(*argv[1], sizeof(int));
  assert(seed);

  for (int i = 0; i < *argv[1]; ++i) {
    seed[i] = rand();
  }

  walk(rank, size, seed);
  printf("rank %d\n", rank);

  MPI_Finalize();
  return 0;
}