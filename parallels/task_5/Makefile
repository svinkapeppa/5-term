build:
	mpicc rw.c -o run
	mpicc simple.c -o simple

test:
	mpirun -n 4 ./simple 10 2 2 100 10 0.25 0.25 0.25 0.25

work:
	mpirun -n 4 ./run 10 2 2 100 10 0.25 0.25 0.25 0.25

debug:
	valgrind --leak-check=full mpirun -n 4 ./run 10 2 2 100 10 0.25 0.25 0.25 0.25