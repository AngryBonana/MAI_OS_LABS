all:
	gcc-14 -c main.c -o main.o
	gcc-14 -c src/readfile.c -o readfile.o
	gcc-14 -c src/batcher_sort.c -o batcher_sort.o
	gcc-14 main.o readfile.o batcher_sort.o -lpthread -O2 -o program