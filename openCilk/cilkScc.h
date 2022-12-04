#include "bfs.h"
#include "mmio.h"
#include "string.h"
#include "pthread.h"
#include "sys/time.h"

#include <cilk/cilk.h>

typedef struct CooArray{
    int* i;
    int* j;

    int iLength;
    int jLength;
    int numOfVertices;
}CooArray;

CooArray* readMtxFile(char* file);

Graph* initGraphFromCoo(CooArray* ca);

int cilkColorScc(Graph* g, bool trimming);