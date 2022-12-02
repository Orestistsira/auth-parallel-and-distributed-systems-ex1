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

CooArray* readMtxFile(char* file);

void calculateVertexDegrees(Graph* g, int startingVertex, int endingVertex);

void trimGraph(Graph* g, int startingVertex, int endingVertex);

Graph* initGraphFromCoo(CooArray* ca);

void initColor(Graph* g, int* vertexColor, int startingVertex, int endingVertex);

Array* findUniqueColors(int* vertexColor, int size);

int sequentialColorScc(Graph* g, bool trimming);