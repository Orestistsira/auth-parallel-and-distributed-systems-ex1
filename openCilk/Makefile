SHELL := /bin/bash

CLANG    = /opt/opencilk/bin/clang
FLAGS  = -O3 -fopencilk -pthread -fcilktool=cilkscale -g

cilk: cilkMain.c cilkScc.c bfs.c graph.c mmio.c
	$(CLANG) $(FLAGS) cilkMain.c cilkScc.c bfs.c graph.c mmio.c -o cilkScc

clean:
	$(RM) cilkScc