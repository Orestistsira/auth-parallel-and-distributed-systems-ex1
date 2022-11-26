#include "bfs.h"
#include "mmio.h"
#include "string.h"
#include "pthread.h"

#include <omp.h>

typedef struct CooArray{
    int* i;
    int* j;

    int iLength;
    int jLength;
    int numOfVertices;
}CooArray;

bool notInArray(int* arr, int size, int value);

CooArray* readMtxFile(char* file);

Graph* initGraphFromCoo(CooArray* ca);

int openmpColorScc(Graph* g, bool trimming);