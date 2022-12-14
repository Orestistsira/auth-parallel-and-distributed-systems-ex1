#include "parallelScc.h"

typedef struct Arguments{
    Graph* g;
    Array* uc;
    int* vertexColor;
    int startIndex;
    int endIndex;
}Arguments;

typedef struct TrimArguments{
    Graph* g;
    int startIndex;
    int endIndex;
    int timesFoundInStart;
    int timesFoundInEnd;
}TrimArguments;

//global vars
int parSccCounter;
bool parChangedColor;
int numOfThreads;

pthread_attr_t attr;
pthread_mutex_t mutexAddScc;

void* parCalculateVertexDegrees(void* args){
    Arguments* arguments = (Arguments*) args;
    Graph* g = arguments->g;
    int startingVertex = arguments->startIndex;
    int endingVertex = arguments->endIndex;

    calculateVertexDegrees(g, startingVertex, endingVertex);

    pthread_exit(NULL);
}

void* parTrimGraph(void* args){
    Arguments* arguments = (Arguments*) args;
    Graph* g = arguments->g;
    int startingVertex = arguments->startIndex;
    int endingVertex = arguments->endIndex;

    //For every vertex ID in vertices array of graph 
    for(int i=startingVertex;i<endingVertex;i++){
        if(g->vertices[i] == -1) continue;

        //If the in-degree or out-degree is zero trim the vertex
        if(g->inDegree[i] == 0 || g->outDegree[i] == 0){
            deleteIndexfromArray(g->vertices, i);

            pthread_mutex_lock(&mutexAddScc);
            g->sccIdOfVertex[i] = parSccCounter++;
            g->numOfVertices--;
            pthread_mutex_unlock(&mutexAddScc);
        }

        g->inDegree[i] = 0;
        g->outDegree[i] = 0;
    }    

	pthread_exit(NULL);
}

void createThreadsForTrim(Graph* g){
    pthread_t thread[numOfThreads];

    int rc;
	long i;
    int local;

    Arguments arguments[numOfThreads];

    //Calculate degrees of each vertex
    local = g->endLength / numOfThreads + 1;
	for(i=0;i<numOfThreads;i++){

        Arguments* args = &arguments[i];
        args->g = g;

        args->startIndex = i * local;
        args->endIndex = ((i + 1) * local) > g->endLength ? g->endLength : (i + 1) * local;

        rc = pthread_create(&thread[i], &attr, parCalculateVertexDegrees, (void*)args);

		if(rc){
			printf("Error in thread #%ld! Return code from pthread_create() is %d\n", i, rc);
			exit(-1);
		}
    }

	for(i=0;i<numOfThreads;i++){
		rc = pthread_join(thread[i], NULL);

		if(rc){
			printf("ERROR; return code from pthread_join() is %d\n", rc);
			exit(-1);
		}
	}


    //Trim graph
    local = g->verticesLength / numOfThreads + 1;
	for(i=0;i<numOfThreads;i++){

        Arguments* args = &arguments[i];
        args->g = g;

        args->startIndex = i * local;
        args->endIndex = ((i + 1) * local) > g->verticesLength ? g->verticesLength : (i + 1) * local;

        rc = pthread_create(&thread[i], &attr, parTrimGraph, (void*)args);

		if(rc){
			printf("Error in thread #%ld! Return code from pthread_create() is %d\n", i, rc);
			exit(-1);
		}
    }

	for(i=0;i<numOfThreads;i++){
		rc = pthread_join(thread[i], NULL);

		if(rc){
			printf("ERROR; return code from pthread_join() is %d\n", rc);
			exit(-1);
		}
	}
}

//Initializes each vertex with a color which equals to its ID
void* parInitColor(void* args){
    Arguments* arguments = (Arguments*) args;
    Graph* g = arguments->g;
    int* vertexColor = arguments->vertexColor;
    int startingVertex = arguments->startIndex;
    int endingVertex = arguments->endIndex;
  
    initColor(g, vertexColor, startingVertex, endingVertex);

    pthread_exit(NULL);
}

void createThreadsForInitColor(Graph* g, int* vertexColor){
    pthread_t thread[numOfThreads];

    int rc;
	long i;
    int local;

    Arguments arguments[numOfThreads];

    local = g->verticesLength / numOfThreads + 1;
    for(i=0;i<numOfThreads;i++){

        Arguments* args = &arguments[i];
        args->g = g;

        args->vertexColor = vertexColor;
        args->startIndex = i * local;
        args->endIndex = ((i + 1) * local) > g->verticesLength ? g->verticesLength : (i + 1) * local;

        rc = pthread_create(&thread[i], &attr, parInitColor, (void*)args);

        if(rc){
            printf("Error in thread #%ld! Return code from pthread_create() is %d\n", i, rc);
            exit(-1);
        }
    }

    for(i=0;i<numOfThreads;i++){
        rc = pthread_join(thread[i], NULL);
        if(rc){
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
    }
}

//Spreads color forward following the path of the edges
void* parSpreadColor(void* args){
    Arguments* arguments = (Arguments*) args;
    Graph* g = arguments->g;
    int* vertexColor = arguments->vertexColor;
    int startingVertex = arguments->startIndex;
    int endingVertex = arguments->endIndex;

    for(int i=startingVertex;i<endingVertex;i++){
        int vid = g->vertices[i];
        if(vid == -1) continue;

        int color = vertexColor[vid];
        if(color == 0) continue;

        int startIndex = g->vertexPosInStart[vid];

        //Follow the edges and spread color to the end vertices
        int ifinish = startIndex + 1 < g->startPointerLength ? g->startPointer[startIndex+1] : g->endLength;

        for(int endIndex=g->startPointer[startIndex];endIndex<ifinish;endIndex++){
            int endvid = g->vertices[g->end[endIndex]];
            //If vertex has been removed
            if(endvid == -1) continue;

            int nextColor = vertexColor[endvid];

            if(nextColor < color){
                vertexColor[vid] = vertexColor[endvid];
                parChangedColor = true;
            }
        }
    }

    pthread_exit(NULL);
}

void createThreadsForSpreadColor(Graph* g, int* vertexColor){
    pthread_t thread[numOfThreads];

    int rc;
	long i;
    int local;

    Arguments arguments[numOfThreads];
    
    local = g->verticesLength / numOfThreads + 1;
    for(i=0;i<numOfThreads;i++){

        Arguments* args = &arguments[i];
        args->g = g;

        args->vertexColor = vertexColor;
        args->startIndex = i * local;
        args->endIndex = ((i + 1) * local) > g->verticesLength ? g->verticesLength : (i + 1) * local;

        rc = pthread_create(&thread[i], &attr, parSpreadColor, (void*)args);

        if(rc){
            printf("Error in thread #%ld! Return code from pthread_create() is %d\n", i, rc);
            exit(-1);
        }
    }

    for(i=0;i<numOfThreads;i++){
        rc = pthread_join(thread[i], NULL);
        if(rc){
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
    }
}

void* parAccessUniqueColors(void* args){
    Arguments* arguments = (Arguments*) args;
    Graph* g = arguments->g;
    int* vertexColor = arguments->vertexColor;
    Array* uc = arguments->uc;
    int startingVertex = arguments->startIndex;
    int endingVertex = arguments->endIndex;

    int n = g->verticesLength;
    Queue* queue = (Queue*) malloc(sizeof(Queue));
    if(queue == NULL)
        printf("ERROR in queue malloc");

    queueInit(queue, n);

    Array* scc = (Array*) malloc(sizeof(Array));
    if(scc == NULL)
        printf("ERROR in scc malloc");
    scc->arr = (int*) malloc(n * sizeof(int));
    if(scc->arr == NULL)
        printf("ERROR in scc array malloc");
    scc->length = 0;

    for(int i=startingVertex;i<endingVertex;i++){
        int color = uc->arr[i];

        queue->qStart = 0;
        queue->qEnd = 0;
        scc->length = 0;

        //Find all vertices included to the scc for the current unique color
        bfs(g, color, vertexColor, queue, scc);

        //Count SCCs found and delete from graph all vertices contained in a SCC
        if(scc->length > 0){

            //Delete each vertex with if found in scc
            for(int j=0;j<scc->length;j++){
                int vid = scc->arr[j];
                deleteVertexFromGraph(g, vid);

                g->sccIdOfVertex[vid] = parSccCounter;
            }
            //MUTEX
            pthread_mutex_lock(&mutexAddScc);
            parSccCounter++;
            g->numOfVertices -= scc->length;
            pthread_mutex_unlock(&mutexAddScc);
        }
        else{
            printf("Error: Did not find any SCCs for color=%d!\n", color);
            exit(1);
        }

    }

    free(scc->arr);
    free(scc);
    free(queue->arr);
    free(queue);

    pthread_exit(NULL);
}

void createThreadsForUniqueColor(Graph* g, Array* uc, int* vertexColor){
    pthread_t thread[numOfThreads];

    int rc;
	long i;
    int local;

    Arguments arguments[numOfThreads];

    local = uc->length / numOfThreads + 1;
    for(i=0;i<numOfThreads;i++){
        Arguments* args = &arguments[i];
        args->g = g;
        args->uc = uc;
        args->vertexColor = vertexColor;
        args->startIndex = i * local;
        args->endIndex = ((i + 1) * local) > uc->length ? uc->length : (i + 1) * local;
            
        rc = pthread_create(&thread[i], &attr, parAccessUniqueColors, (void*)args);

        if(rc){
            printf("Error in thread #%ld! Return code from pthread_create() is %d\n", i, rc);
            exit(-1);
        }
    }

    for(i=0;i<numOfThreads;i++){
        rc = pthread_join(thread[i], NULL);
        if(rc){
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
    }
}

int parallelColorScc(Graph* g, bool trimming, int givenNumOfThreads){
    //Init number of threads
    numOfThreads = givenNumOfThreads;
    printf("Number of threads=%d\n", numOfThreads);
    parSccCounter = 0;

    //Initialize mutex
    pthread_mutex_init(&mutexAddScc, NULL);

    //Initialize and set thread detached attribute to joinable
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    //Init VertexColor array
    //Each Index corresponds to de vertices array and the value is the color of the vertex
    int n = g->verticesLength;
    int* vertexColor = (int*) malloc(n * sizeof(int));
    if(vertexColor == NULL)
        printf("ERROR in vertexColor malloc\n");

    while(g->numOfVertices > 0){
        //Trim Graph
        if(trimming){
            createThreadsForTrim(g);
        }

        //Init each vertex color cell with the id of each vertex
        createThreadsForInitColor(g, vertexColor);

        //Spread vertex color fw until there are no changes in vertexColor
        parChangedColor = true;
        while(parChangedColor){
            
            parChangedColor = false;
            createThreadsForSpreadColor(g, vertexColor);
        }    

        //Find all unique colors left in the vertexColor array
        Array* uc = findUniqueColors(vertexColor, n);

        //For each unique color, do BFS for the for the subgraph with that color
        createThreadsForUniqueColor(g, uc, vertexColor);

        free(uc->arr);
        free(uc);
    }

    free(vertexColor);

    pthread_mutex_destroy(&mutexAddScc);
    pthread_attr_destroy(&attr);

    return parSccCounter;
}