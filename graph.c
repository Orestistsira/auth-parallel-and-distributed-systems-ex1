#include "graph.h"

//Deletes index by setting its value to -1
void deleteIndexfromArray(int*arr, int index){
    arr[index] = -1;
}

//Deletes vertex by setting its id to -1
void deleteVertexFromGraph(Graph* g, int vid){
    if(g->vertices != NULL){
        for(int i=0;i<g->verticesLength;i++){
            if(g->vertices[i] == vid){
                g->vertices[i] = -1;

                return;
            }
        }
    }
}