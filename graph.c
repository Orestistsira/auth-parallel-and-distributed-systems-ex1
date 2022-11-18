#include "graph.h"

void deleteIndexfromArray(int*arr, int index){
    arr[index] = -1;
}

void deleteVertexFromGraph(Graph* g, int* vertexColor, int vid){
    if(g->vertices != NULL){
        for(int i=0;i<g->verticesLength;i++){
            if(g->vertices[i] == vid){
                g->vertices[i] = -1;
                g->numOfVertices--;

                return;
            }
        }
    }
}