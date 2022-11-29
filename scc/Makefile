SHELL := /bin/bash

CC       = gcc
FLAGS    = -O3 -pthread -g

scc: main.c seqScc.c parallelScc.c bfs.c graph.c mmio.c
	$(CC) $(FLAGS) main.c seqScc.c parallelScc.c bfs.c graph.c mmio.c -o colorScc

clean:
	$(RM) colorScc