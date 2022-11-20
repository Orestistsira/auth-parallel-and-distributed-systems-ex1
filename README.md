# auth-parallel-and-distributed-systems

gcc -Wall main.c seqScc.c parallelScc.c bfs.c graph.c mmio.c -o colorScc -O3

./colorScc [graph name] [sequential/parallel]