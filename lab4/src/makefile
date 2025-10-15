CC=gcc
CFLAGS=-I.

all: parallel_min_max run_sequential process_memory

parallel_sum : utils.o sum_utils.o parallel_sum.c
	$(CC) -o parallel_sum utils.o sum_utils.o parallel_sum.c $(CFLAGS) $(LDFLAGS)

parallel_min_max : utils.o find_min_max.o utils.h find_min_max.h
	$(CC) -o parallel_min_max utils.o find_min_max.o parallel_min_max.c $(CFLAGS)

run_sequential : 
	$(CC) -o run_sequential run_sequential.c $(CFLAGS)

run_rocess_memory : 
	$(CC) -o process_memory process_memory.c $(CFLAGS)

zoombie : 
	$(CC) -o zoombie zoombie.c $(CFLAGS)

utils.o : utils.h
	$(CC) -o utils.o -c utils.c $(CFLAGS)

find_min_max.o : utils.h find_min_max.h
	$(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)

sum_utils.o : sum_utils.h
	$(CC) -o sum_utils.o -c sum_utils.c $(CFLAGS)

clean :
	rm utils.o find_min_max.o parallel_min_max run_sequential process_memory
 