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

//Finds an array with the vertex IDs contained in the SCC
void bfs(Graph* g, int s, int* vertexColor, Queue* queue, Array* sccList){
    
    int n = g->verticesLength;
    //Init visited array for each vertex
    bool* visited = (bool*) malloc(n * sizeof(int));
    int color = s;

    //Mark all vertices as not visited
    for(int i=0;i<n;i++){
        visited[i] = false;
    }

    //Push source vertex to queue
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
}