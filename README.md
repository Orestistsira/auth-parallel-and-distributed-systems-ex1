# auth-parallel-and-distributed-systems

Compilers:

gcc (11.3.0)

clang from OpenCilk-2.0.0 ([Installation Guide](https://www.opencilk.org/doc/users-guide/install/))

---

**Do not skip any arguments unless it is noted**

---

1. To run sequential or Pthreads parallel algorithm 

cd scc

make

./colorScc [graph-name] [trimming/no-trimming] [sequential/parallel] [number-of-threads]

E.g. ./colorScc foldoc trimming parallel 8

*--[number-of-threads] can be skipped, default = 4*

---

2. To run parallel OpenCilk algorithm

cd openCilk

make

[CILK_NWORKERS=N] ./cilkScc [graph-name] [trimming/no-trimming]

E.g. ./cilkScc foldoc trimming

*--[CILK_NWORKERS=N] can be skipped to set OpenCilk's default workers*

---

3. To run parallel openMP algorithm

cd openMP

make

./opempScc [graph-name] [trimming/no-trimming]

E.g. ./openmpScc foldoc trimming
