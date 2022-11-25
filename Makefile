SHELL := /bin/bash

CC       = gcc
FLAGS    = -O3 -pthread

CLANG    = /opt/opencilk/bin/clang
C_FLAGS  = -O3 -fopencilk -pthread -fcilktool=cilkscale

scc: main.c seqScc.c parallelScc.c bfs.c graph.c mmio.c
	$(CC) $(FLAGS) main.c seqScc.c parallelScc.c bfs.c graph.c mmio.c -o colorScc

cilk: cilkMain.c cilkScc.c bfs.c graph.c mmio.c
	$(CLANG) $(C_FLAGS) cilkMain.c cilkScc.c bfs.c graph.c mmio.c -o cilkScc

clean:
	$(RM) colorScc