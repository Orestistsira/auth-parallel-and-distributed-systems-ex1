SHELL := /bin/bash

CC       = gcc
FLAGS    = -O3 -fopenmp

openmp: openmpMain.c openmpScc.c bfs.c graph.c mmio.c
	$(CC) $(FLAGS) openmpMain.c openmpScc.c bfs.c graph.c mmio.c -o openmpScc

clean:
	$(RM) openmpScc