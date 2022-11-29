#include "bfs.h"

//Initialize queue
void queueInit(Queue* queue, int size){
    queue->qSize = size;
    queue->arr = (int*) malloc(queue->qSize * sizeof(int));
    if(queue->arr == NULL)
        printf("ERROR in queue array malloc");
    queue->qStart = 0;
    queue->qEnd = 0;
}

//Push value at the end of queue
void queuePush(Queue* queue, int val){
    queue->arr[queue->qEnd] = val;
    queue->qEnd++;
    if(queue->qEnd == queue->qSize)
        queue->qEnd = 0;
}

//Returns the value at the start of the queue
int queuePop(Queue* queue){
    int val = queue->arr[queue->qStart];
    queue->qStart++;
    if(queue->qStart == queue->qSize)
        queue->qStart = 0;
    return val;
}

//Returns the index of a certain value in the given array
int getIndexOfValue(int* array, int n, int value){
    for(int i=0;i<n;i++){
        if(array[i] == value){
            return i;
        }
    }
    //printf("Index for value %d not found\n", value);
    return -1;
}

//Resizes array to a new given size
void resizeArray(int* arr, int newSize){
    int* temp = (int*) realloc(arr, newSize * sizeof(int));
    arr = temp;
}

//Checks if value is not contained in the given array
bool notInArray(int* arr, int size, int value){
    for(int i=0;i<size;i++){
        if(arr[i] == value)
            return false;
    }
    return true;
}

//Returns an array with the vertex IDs contained in the SCC
void bfs(Graph* g, int s, int* vertexColor, Queue* queue, Array* sccList){
    
    int n = g->verticesLength;
    //For each vertex
    // bool visited[n];
    bool* visited = (bool*) malloc(n * sizeof(int));
    int color = s;

    //Mark all vertices as not visited
    for(int i=0;i<n;i++){
        visited[i] = false;
    }

    //Create a queue for bfs
    if(queue->arr == 0)
        printf("ERROR in queue array init");
    queuePush(queue, s);

    //Mark source vertex as visited
    int indexOfVertex = s;
    visited[indexOfVertex] = true;

    while(queue->qStart != queue->qEnd){
        //Dequeue the first vertex from queue
        int s = queuePop(queue);

        //Put the vertex id on the ssc list
        sccList->arr[sccList->length] = s;
        sccList->length++;

        //Get all the adjacent vertices of s and enqueue them if not visited
        //TODO: store start index of each vertex from init
        //int startIndex = getIndexOfValue(g->start, g->startLength, s);
        int startIndex = g->vertexPosInStart[s];

        //if vertex is a start of an edge
        if(startIndex != -1){
            int ifinish = startIndex + 1 < g->startPointerLength ? g->startPointer[startIndex+1] : g->endLength;

            for(int endIndex=g->startPointer[startIndex];endIndex<ifinish;endIndex++){
                int endvid = g->vertices[g->end[endIndex]];
                if(endvid == -1 || vertexColor[endvid] != color){
                    continue;
                }

                int indexOfVertex = endvid;

                if(visited[indexOfVertex] == false){
                    queuePush(queue, g->vertices[indexOfVertex]);
                    visited[indexOfVertex] = true;
                }
            }
        }
    }

    free(visited);

    //resizeArray(sccList->arr, sccList->length);

    //printf("end bfs\n");
}

Array* cilkBfs(Graph* g, int s){
    int n = g->verticesLength;
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
                    printf("Error: Vertex with id=%d not found\n", g->end[endIndex]);
                    continue;
                }

                if(visited[indexOfVertex] == false){
                    queuePush(queue, g->vertices[indexOfVertex]);
                    visited[indexOfVertex] = true;
                }
            }
        }
    }

    return sccList;
}