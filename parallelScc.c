#include "parallelScc.h"

typedef struct Arguments{
    int id;
    Graph* g;
    Array* uc;
    int* vertexColor;
    int startIndex;
    int endIndex;
}Arguments;

//global vars
int parSccCounter;
bool parChangedColor;

pthread_attr_t attr;
pthread_mutex_t mutexDeleteVertex;
pthread_mutex_t mutexChangeVertexColor;

//Identifies and removes all trivial SCCs
void* parTrimGraph(void* args){
    //Graph* g, int startingVertex, int endingVertex
    Arguments* arguments = (Arguments*) args;
    Graph* g = arguments->g;
    int startingVertex = arguments->startIndex;
    int endingVertex = arguments->endIndex;
    int threadId = arguments->id;

    printf("Start of thread %d:%d\n", threadId, startingVertex);
    printf("End of thread %d:%d\n", threadId, endingVertex);

    //For every vertex ID in vertices array of graph 
    for(int i=startingVertex;i<endingVertex;i++){
        int vid = g->vertices[i];

        //Check if the vertex with this ID is a start of an edge
        int timesFoundInStart = 0;
        for(int startIndex=0;startIndex<g->startLength;startIndex++){
            if(g->start[startIndex] == vid){
                timesFoundInStart++;
                break;
            }
        }

        //Check if the vertex with this ID is an end of an edge
        int timesFoundInEnd = 0;
        for(int endIndex=0;endIndex<g->startLength;endIndex++){
            if(g->start[endIndex] == vid){
                timesFoundInEnd++;
                break;
            }
        }

        //If the in-degree or out-degree is zero trim the vertex
        if(timesFoundInEnd == 0 || timesFoundInStart == 0){
            //printf("Trimming vertex: %d\n", vid);
            pthread_mutex_lock(&mutexDeleteVertex);
            deleteIndexfromArray(g->vertices, i);     
            parSccCounter++;
            g->numOfVertices--;
            pthread_mutex_unlock(&mutexDeleteVertex);
        }
    }

    resizeArray(g->vertices, g->verticesLength);

    /* Arguments go to status */
	pthread_exit(NULL);
}

//Initializes each vertex with a color which equals to its ID
void* parInitColor(void* args){
    //Graph* g, int* vertexColor, int startingVertex, int endingVertex
    Arguments* arguments = (Arguments*) args;
    Graph* g = arguments->g;
    int* vertexColor = arguments->vertexColor;
    int startingVertex = arguments->startIndex;
    int endingVertex = arguments->endIndex;
    int threadId = arguments->id;

    printf("Start of thread %d:%d\n", threadId, startingVertex);
    printf("End of thread %d:%d\n", threadId, endingVertex);

    for(int i=startingVertex;i<endingVertex;i++){
        vertexColor[i] = -1;

        int vid = g->vertices[i];
        if(vid != -1)
            vertexColor[i] = vid;
    }   

    pthread_exit(NULL); 
}

//Spreads color forward following the path of the edges
void* parSpreadColor(void* args){
    //Graph* g, int* vertexColor, int startingVertex, int endingVertex
    Arguments* arguments = (Arguments*) args;
    Graph* g = arguments->g;
    int* vertexColor = arguments->vertexColor;
    int startingVertex = arguments->startIndex;
    int endingVertex = arguments->endIndex;
    int threadId = arguments->id;

    printf("Start of thread %d:%d\n", threadId, startingVertex);
    printf("End of thread %d:%d\n", threadId, endingVertex);

    for(int i=startingVertex;i<endingVertex;i++){
        int vid = g->vertices[i];
        if(vid != -1){
            int color = vertexColor[i];
            int vid = g->vertices[i];

            //Find the index of the origin vertex in the start array
            int startIndex = getIndexOfValue(g->start, g->startLength, vid);

            //if vertex is a start of an edge
            if(startIndex != -1){
                //Follow the edges and spread color to the end vertices
                int ifinish = startIndex + 1 < g->startPointerLength ? g->startPointer[startIndex+1] : g->endLength;

                for(int endIndex=g->startPointer[startIndex];endIndex<ifinish;endIndex++){
                    //If vertex has been removed
                    int endvid = g->end[endIndex];
                    if(endvid == -1){
                        continue;
                    }

                    int nextColorIndex = getIndexOfValue(g->vertices, g->verticesLength, endvid);
                    //If vertex index was not found
                    if(nextColorIndex == -1){
                        continue;
                    }

                    int nextColor = vertexColor[nextColorIndex];

                    if(nextColor < color){
                        pthread_mutex_lock(&mutexChangeVertexColor);
                        vertexColor[i] = vertexColor[nextColorIndex];
                        parChangedColor = true;
                        pthread_mutex_unlock(&mutexChangeVertexColor);
                    }
                }
            }
        } 
    }

    pthread_exit(NULL);
}

void* parAccessUniqueColors(void* args){
    //Graph* g, Array* uc, int* vertexColor, int startingColor, int endingColor

    Arguments* arguments = (Arguments*) args;
    Graph* g = arguments->g;
    int* vertexColor = arguments->vertexColor;
    Array* uc = arguments->uc;
    int startingVertex = arguments->startIndex;
    int endingVertex = arguments->endIndex;
    int threadId = arguments->id;

    printf("Start of thread %d:%d\n", threadId, startingVertex);
    printf("End of thread %d:%d\n", threadId, endingVertex);

    for(int i=startingVertex;i<endingVertex;i++){
        // printf("Vertex Color: ");
        // printArray(vertexColor, g->verticesLength);

        int color = uc->arr[i];

        // printf("Color:%d\n", color);
        //Find all vertexes with color and put them in vc
        Array* scc = findSccOfColor(g, vertexColor, color);

        // printf("SccLength=%d", scc->length);
        //Count SCCs found and delete from graph all vertices contained in a SCC
        if(scc->length > 0){
            parSccCounter++;

            //Delete each vertex with if found in scc
            for(int j=0;j<scc->length;j++){
                int vid = scc->arr[j];
                pthread_mutex_lock(&mutexDeleteVertex);
                deleteVertexFromGraph(g, vertexColor, vid);
                pthread_mutex_unlock(&mutexDeleteVertex);
            }
        }
        else{
            printf("Error: Did not find any SCCs for color=%d!\n", color);
            exit(1);
        }
        free(scc);
    }

    pthread_exit(NULL);
}

int numOfThreads = 10;

int parallelColorScc(Graph* g){
    //Init threads
    pthread_t thread[numOfThreads];

    int rc;
	long i;
    int local;

    parSccCounter = 0;
    printf("NumOfVertices=%d\n", g->numOfVertices);
    
    printf("Trimming...\n");
    //Trim trvial SCCs to simplify the graph
    //Can be done in parallel

    //Initialize mutexes
    pthread_mutex_init(&mutexDeleteVertex, NULL);
    pthread_mutex_init(&mutexChangeVertexColor, NULL);

    //Initialize and set thread detached attribute to joinable
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    local = g->verticesLength / numOfThreads + 1;
	for(i=0;i<numOfThreads;i++){
        printf("In parallelScc: Creating thread #%ld\n", i);

        Arguments* args = (Arguments*) malloc(sizeof(Arguments));
        args->g = g;
        args->id = i;

        args->startIndex = i * local;
        args->endIndex = ((i + 1) * local) > g->verticesLength ? g->verticesLength : (i + 1) * local;

        rc = pthread_create(&thread[i], &attr, parTrimGraph, (void*)args);

		if(rc){
			printf("Error in thread #%ld! Return code from pthread_create() is %d\n", i, rc);
			exit(-1);
		}
    }
    /* Free attribute and wait for the other threads */
	pthread_attr_destroy(&attr);

	for(i=0;i<numOfThreads;i++){
		rc = pthread_join(thread[i], NULL);
		if(rc){
			printf("ERROR; return code from pthread_join() is %d\n", rc);
			exit(-1);
		}
		printf("Main: completed join with thread %ld\n", i);
	}
    printf("Trimming ended\n");

    //Init VertexColor array
    //Each Index corresponds to de vertices array and the value is the color of the vertex
    int n = g->verticesLength;
    int* vertexColor = (int*) malloc(n * sizeof(int));

    while(g->numOfVertices > 0){
        numOfThreads = 10;
        if(g->numOfVertices == 1){
            parSccCounter++;
            break;
        }

        printf("Start\n");
        // printGraph(g);
        printf("NumOfVertices=%d\n", g->numOfVertices);

        //Init each vertex color withe the vertex id
        //Can be done in Parallel
        printf("Initializing colors...\n");
        local = g->verticesLength / numOfThreads + 1;
        for(i=0;i<numOfThreads;i++){
            printf("In parallelScc: Creating thread #%ld\n", i);

            Arguments* args = (Arguments*) malloc(sizeof(Arguments));
            args->g = g;
            args->id = i;

            args->vertexColor = vertexColor;
            args->startIndex = i * local;
            args->endIndex = ((i + 1) * local) > g->verticesLength ? g->verticesLength : (i + 1) * local;

            rc = pthread_create(&thread[i], &attr, parInitColor, (void*)args);

            if(rc){
                printf("Error in thread #%ld! Return code from pthread_create() is %d\n", i, rc);
                exit(-1);
            }
        }
        /* Free attribute and wait for the other threads */
        pthread_attr_destroy(&attr);

        for(i=0;i<numOfThreads;i++){
            rc = pthread_join(thread[i], NULL);
            if(rc){
                printf("ERROR; return code from pthread_join() is %d\n", rc);
                exit(-1);
            }
            printf("Main: completed join with thread %ld\n", i);
        }
        //parInitColor(g, vertexColor, 0, n);
        printf("Colors initialized...\n");
        // printf("Vertex Color: ");
        // printArray(vertexColor, g->verticesLength);

        //Spread vertex color fw until there are no changes in vertexColor
        parChangedColor = true;
        while(parChangedColor){
            printf("Spreading color...\n");
            parChangedColor = false;
            
            //Can be done in Parallel
            local = g->verticesLength / numOfThreads + 1;
            for(i=0;i<numOfThreads;i++){
                printf("In parallelScc: Creating thread #%ld\n", i);

                Arguments* args = (Arguments*) malloc(sizeof(Arguments));
                args->g = g;
                args->id = i;

                args->vertexColor = vertexColor;
                args->startIndex = i * local;
                args->endIndex = ((i + 1) * local) > g->verticesLength ? g->verticesLength : (i + 1) * local;

                rc = pthread_create(&thread[i], &attr, parSpreadColor, (void*)args);

                if(rc){
                    printf("Error in thread #%ld! Return code from pthread_create() is %d\n", i, rc);
                    exit(-1);
                }
            }
            /* Free attribute and wait for the other threads */
            pthread_attr_destroy(&attr);

            for(i=0;i<numOfThreads;i++){
                rc = pthread_join(thread[i], NULL);
                if(rc){
                    printf("ERROR; return code from pthread_join() is %d\n", rc);
                    exit(-1);
                }
                printf("Main: completed join with thread %ld\n", i);
            }
            //parSpreadColor(g, vertexColor, 0, n);
        }
        
        printf("Spreading color ended\n");

        // printf("Vertex Color: ");
        // printArray(vertexColor, g->verticesLength);

        //Find all unique colors left in the vertexColor array
        //Can be done in parallel?
        Array* uc = findUniqueColors(vertexColor, n);

        printf("Number of Unique colors=%d\n", uc->length);

        printf("Finding scc number...\n");
        //For each unique color, do BFS for the for the subgraph with that color
        //Can be done in parallel
        local = uc->length / numOfThreads + 1;
        printf("Local=%d\n", local);
        for(i=0;i<numOfThreads;i++){
            printf("In parallelScc: Creating thread #%ld\n", i);

            Arguments* args = (Arguments*) malloc(sizeof(Arguments));
            args->g = g;
            args->id = i;
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
        /* Free attribute and wait for the other threads */
        pthread_attr_destroy(&attr);

        for(i=0;i<numOfThreads;i++){
            rc = pthread_join(thread[i], NULL);
            if(rc){
                printf("ERROR; return code from pthread_join() is %d\n", rc);
                exit(-1);
            }
            printf("Main: completed join with thread %ld\n", i);
        }
        //parAccessUniqueColors(g, uc, vertexColor, 0, uc->length);

        printf("NumOfVertices=%d\n", g->numOfVertices);
        printf("SCCs found=%d\n", parSccCounter);
        free(uc);
    }

    return parSccCounter;
    pthread_exit(NULL);
    pthread_mutex_destroy(&mutexDeleteVertex);
    pthread_mutex_destroy(&mutexChangeVertexColor);
}