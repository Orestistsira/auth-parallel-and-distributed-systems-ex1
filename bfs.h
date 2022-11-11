#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

#include "queue.h"
#include "graph.h"

typedef struct SccList{
    int* arr;
    int length;
}SccList;

int getIndexOfValue(int* array, int n, int value);

SccList* bfs(Graph* g, int s);