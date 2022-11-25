#include "bfs.h"
#include "mmio.h"
#include "string.h"
#include <sys/time.h>

typedef struct CooArray{
    int* i;
    int* j;

    int iLength;
    int jLength;
    int numOfVertices;
}CooArray;

bool notInArray(int* arr, int size, int value);

CooArray* readMtxFile(char* file);

void trimGraph(Graph* g, int startingVertex, int endingVertex);

Graph* initGraphFromCoo(CooArray* ca);

void initColor(Graph* g, int* vertexColor, int startingVertex, int endingVertex);

void spreadColor(Graph* g, int* vertexColor, int startingVertex, int endingVertex);

Array* findUniqueColors(int* vertexColor, int size);

Graph* createSubgraph(Graph* g, int* vc, int vcLength);

void accessUniqueColors(Graph* g, Array* uc, int* vertexColor, int startingColor, int endingColor);

int sequentialColorScc(Graph* g, bool trimming);