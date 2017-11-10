build:
	mpicc simple.c -o simple

test:
	mpirun -n 4 ./simple 10 2 2 100 10 0.25 0.25 0.25 0.25

debug:
	mpirun -n 4 valgrind --leak-check=full ./simple 10 2 2 100 10 0.25 0.25 0.25 0.25