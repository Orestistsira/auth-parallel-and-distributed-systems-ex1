#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

#include "queue.h"
#include "graph.h"

typedef struct Array{
    int* arr;
    int length;
}Array;

int getIndexOfValue(int* array, int n, int value);

Array* bfs(Graph* g, int s);