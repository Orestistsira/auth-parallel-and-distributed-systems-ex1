#include <stdio.h>
#include <stdlib.h>

typedef struct Graph{
    int* vertices;

    int* startPointer; //points to where start values begin
    int* end;
    int* start;

    int* inDegree;
    int* outDegree;

    int verticesLength;
    int endLength;
    int startLength;
    int startPointerLength;

    //number of vertices not -1
    int numOfVertices;

}Graph;

void deleteIndexfromArray(int*arr, int index);

void deleteVertexFromGraph(Graph* g, int* vertexColor, int vid);