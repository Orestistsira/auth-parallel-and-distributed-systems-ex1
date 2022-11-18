#include "bfs.h"
#include "mmio.h"
#include "string.h"

typedef struct CooArray{
    int* i;
    int* j;

    int iLength;
    int jLength;
    int numOfVertices;
}CooArray;

void printArray(int* array, int n);

void printGraph(Graph* g);

int* copyArray(int* arr, int size);

bool notInArray(int* arr, int size, int value);

void bfsTest(Graph* g, int source);

CooArray* readMtxFile(char* file);

int trimGraph(Graph* g);

Graph* initGraphFromCoo(CooArray* ca);

void initColor(int* vertexColor, int index, int color);

void spreadColor(Graph* g, int i, int* vertexColor, bool* changedColor);

Array* findUniqueColors(int* vertexColor, int size);

Graph* createSubgraph(Graph* g, int* vc, int vcLength);

Array* findSccOfColor(Graph* g, int* vertexColor, int color);

int sequentialColorScc(Graph* g);