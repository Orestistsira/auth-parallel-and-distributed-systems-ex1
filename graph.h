#include <stdio.h>
#include <stdlib.h>

typedef struct Graph{
    int* vertices;
    int* startPointer; //shows where start values go

    int* end;
    int* start;

    int numOfVertices;
    int endLength;
    int startLength;

}Graph;

void deleteIndexfromArray(int*arr, int* size, int index);

void deleteVertexFromGraph(Graph* g, int vid);