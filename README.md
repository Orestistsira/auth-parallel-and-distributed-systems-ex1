# auth-parallel-and-distributed-systems

Compilers:

gcc (11.3.0)

clang from OpenCilk-2.0.0 ([Installation Guide](https://www.opencilk.org/doc/users-guide/install/))

---

1. Directory scc (cd scc)

make

./colorScc [graph-name] [trimming/no-trimming] [sequential/parallel] [number-of-threads]

---

2. Directory openCilk (cd openCilk)

make

[CILK_NWORKERS=N] ./cilkScc [graph-name] [trimming/no-trimming]

---

3. Directory openMP (cd openMP)

make

./opempScc [graph-name] [trimming/no-trimming]
