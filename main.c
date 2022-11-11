#include "bfs.h"

void printArray(int* array, int n){
    for(int i=0;i<n;i++){
        printf("%d ", array[i]);
    }
    printf("\n");
}

void bfsTest(Graph* g){
    SccList* sccList = bfs(g, 2);

    printf("Result: ");
    printArray(sccList->arr, sccList->length);

    free(sccList);
}

int main(int argc, char** argv){
    Graph* g = (Graph*) malloc(sizeof(Graph));

    g->numOfVertices = 4;
    g->endLength = 6;
    g->startLength = 4;

    //Init Graph
    g->end = (int[6]){1, 2, 2, 0, 3, 3};
    g->start = (int[4]){0, 1, 2, 3};
    g->startPointer = (int[4]){0, 2, 3, 5};
    g->vertices = (int[4]){0, 1, 2, 3};

    printArray(g->end, g->endLength);
    printArray(g->start, g->startLength);

    //bfsTest(g);

    free(g);

    return 0;
}