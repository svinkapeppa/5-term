build: main.c create.c graph.h thread_pool.h random.h genetic.h
	gcc -pthread main.c -o run graph.c thread_pool.c random.c genetic.c
	gcc create.c -o help graph.c

test:
	./help 20

work:
	./run 4 1000 10 --generate 20