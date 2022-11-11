#include <stdlib.h>

typedef struct Queue{
    int* arr;
    int qSize;
    int qStart;
    int qEnd;
}Queue;

void queueInit(Queue* queue, int size);

void queuePush(Queue* queue, int val);

int queuePop(Queue* queue);