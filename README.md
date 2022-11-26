# auth-parallel-and-distributed-systems

make scc

./colorScc graph-name [trimming/no-trimming] [sequential/parallel]

(default = no-trimming, default = sequential, default = 10 threads)


make cilk

./cilkScc graph-name


make openmp

./opempScc graph-name