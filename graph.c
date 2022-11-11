#include "graph.h"

void deleteIndexfromArray(int*arr, int* size, int index){
    for(int j=index;j<*size-1;j++){
        arr[j] = arr[j+1];
    }
    *size -= 1;
}

void deleteVertexFromGraph(Graph* g, int vid){
    if(g->vertices != NULL){
        for(int i=0;i<g->numOfVertices;i++){
            if(g->vertices[i] == vid){
                deleteIndexfromArray(g->vertices, &g->numOfVertices, i);
            }
        }
    }
}