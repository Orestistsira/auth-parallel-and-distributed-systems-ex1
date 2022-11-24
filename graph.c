#include "graph.h"

//Deletes index by setting its value to -1
void deleteIndexfromArray(int*arr, int index){
    arr[index] = -1;
}

//Deletes vertex by setting its id to -1
void deleteVertexFromGraph(Graph* g, int vid){
    g->vertices[vid] = -1;
}