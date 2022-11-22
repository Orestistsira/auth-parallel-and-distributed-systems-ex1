# auth-parallel-and-distributed-systems

gcc -Wall main.c seqScc.c parallelScc.c bfs.c graph.c mmio.c -O3 -pthread -o colorScc

./colorScc [graph name] [sequential/parallel]