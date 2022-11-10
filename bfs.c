#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

typedef struct Graph{
    int* vertexes;
    int* startPointer; //shows where start values go

    int* end;
    int* start; 

    int numOfVertexes;
    int endLength;
    int startLength;

}Graph;

typedef struct Queue{
    int* arr;
    int qSize;
    int qStart;
    int qEnd;
}Queue;

typedef struct SccList{
    int* arr;
    int length;
}SccList;

void queueInit(Queue* queue, int size){
    queue->qSize = size;
    queue->arr = (int*) malloc(queue->qSize * sizeof(int));
    queue->qStart=0;
    queue->qEnd = 0;
}

void queuePush(Queue* queue, int val){
    queue->arr[queue->qEnd] = val;
    queue->qEnd++;
}

int queuePop(Queue* queue){
    int val = queue->arr[queue->qStart];
    queue->qStart++;
    return val;
}

void printArray(int* array, int n){
    for(int i=0;i<n;i++){
        printf("%d ", array[i]);
    }
    printf("\n");
}

int getIndexOfValue(int* array, int n, int value){
    for(int i=0;i<n;i++){
        if(array[i] == value){
            return value;
        }
    }
    return -1;
}

SccList* bfs(Graph* g, int s){
    int n = g->numOfVertexes;
    //For each vertex
    bool visited[n];

    for(int i=0;i<n;i++){
        visited[i] = false;
    }

    Queue* queue = (Queue*) malloc(sizeof(Queue));
    queueInit(queue, n);
    queuePush(queue, s);
    
    int index = getIndexOfValue(g->vertexes, n, s);
    visited[index] = true;

    SccList* sccList = (SccList*) malloc(sizeof(SccList));
    sccList->arr = (int*) malloc(n * n * sizeof(int));
    sccList->length = 0;

    while(queue->qStart != queue->qEnd){
        s = queuePop(queue);

        sccList->arr[sccList->length] = s;
        sccList->length++;

        index = getIndexOfValue(g->start, g->startLength, s);
        for(int i=g->startPointer[index];i<g->startPointer[index+1];i++){
            int value = getIndexOfValue(g->vertexes, n, g->end[i]);
            if(visited[value] == false){
                queuePush(queue, g->vertexes[value]);
                visited[value] = true;
            }
        }
    }

    return sccList;
}

int main(int argc, char** argv){
    Graph* g = (Graph*) malloc(sizeof(Graph));

    g->numOfVertexes = 4;
    g->endLength = 6;
    g->startLength = 4;

    
    g->end = (int[6]){1, 2, 2, 0, 3, 3};
    g->start = (int[4]){0, 1, 2, 3};
    g->startPointer = (int[4]){0, 2, 3, 5};
    g->vertexes = (int[4]){0, 1, 2, 3};

    printArray(g->end, g->endLength);
    printArray(g->start, g->startLength);

    SccList* sccList = bfs(g, 2);

    printf("Result:");
    printArray(sccList->arr, sccList->length);

    free(g);
    free(sccList);

    return 0;
}