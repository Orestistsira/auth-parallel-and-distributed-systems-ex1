#include "queue.h"

void queueInit(Queue* queue, int size){
    queue->qSize = size;
    queue->arr = (int*) malloc(queue->qSize * sizeof(int));
    queue->qStart = 0;
    queue->qEnd = 0;
}

void queuePush(Queue* queue, int val){
    queue->arr[queue->qEnd] = val;
    queue->qEnd++;
    if(queue->qEnd == queue->qSize)
        queue->qEnd = 0;
}

int queuePop(Queue* queue){
    int val = queue->arr[queue->qStart];
    queue->qStart++;
    if(queue->qStart == queue->qSize)
        queue->qStart = 0;
    return val;
}