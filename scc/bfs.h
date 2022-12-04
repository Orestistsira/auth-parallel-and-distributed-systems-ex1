#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "graph.h"

typedef struct Queue{
    int* arr;
    int qSize;
    int qStart;
    int qEnd;
}Queue;

typedef struct Array{
    int* arr;
    int length;
}Array;

void queueInit(Queue* queue, int size);

void queuePush(Queue* queue, int val);

int queuePop(Queue* queue);

void bfs(Graph* g, int s, int* vertexColor, Queue* queue, Array* sccList);