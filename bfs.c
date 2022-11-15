#include "bfs.h"

int getIndexOfValue(int* array, int n, int value){
    for(int i=0;i<n;i++){
        if(array[i] == value){
            return i;
        }
    }
    //printf("Index for value %d not found\n", value);
    return -1;
}

void resizeArray(int* arr, int newSize){
    int* temp = (int*) realloc(arr, newSize * sizeof(int));
    arr = temp;
}

Array* bfs(Graph* g, int s){
    int n = g->numOfVertices;
    //For each vertex
    bool visited[n];

    //Mark all vertices as not visited
    for(int i=0;i<n;i++){
        visited[i] = false;
    }

    //Create a queue for bfs
    Queue* queue = (Queue*) malloc(sizeof(Queue));
    queueInit(queue, n);
    queuePush(queue, s);

    //Mark source vertex as visited
    int indexOfVertex = getIndexOfValue(g->vertices, n, s);
    visited[indexOfVertex] = true;

    Array* sccList = (Array*) malloc(sizeof(Array));
    sccList->arr = (int*) malloc(n * sizeof(int));
    sccList->length = 0;

    while(queue->qStart != queue->qEnd){
        //Dequeue the first vertex from queue
        s = queuePop(queue);

        //Put the vertex id on the ssc list
        sccList->arr[sccList->length] = s;
        sccList->length++;

        //Get all the adjacent vertices of s and enqueue them if not visited
        int startIndex = getIndexOfValue(g->start, g->startLength, s);

        //if vertex is a start of an edge
        if(startIndex != -1){
            int ifinish = startIndex + 1 < g->startPointerLength ? g->startPointer[startIndex+1] : g->endLength;

            for(int endIndex=g->startPointer[startIndex];endIndex<ifinish;endIndex++){
                indexOfVertex = getIndexOfValue(g->vertices, n, g->end[endIndex]);
                if(indexOfVertex == -1){
                    printf("Error: Vertex with id=%d not found", g->end[endIndex]);
                    continue;
                }

                if(visited[indexOfVertex] == false){
                    queuePush(queue, g->vertices[indexOfVertex]);
                    visited[indexOfVertex] = true;
                }
            }
        }
    }

    resizeArray(sccList->arr, sccList->length);

    return sccList;
}