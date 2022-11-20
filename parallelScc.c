#include "parallelScc.h"

typedef struct Arguments{
    int id;
    Graph* g;
    Array* uc;
    int* vertexColor;
    int startIndex;
    int endIndex;
}Arguments;

typedef struct TrimArguments{
    int vid;
    Graph* g;
    int startIndex;
    int endIndex;
    int timesFoundInStart;
    int timesFoundInEnd;
}TrimArguments;

//global vars
int parSccCounter;
bool parChangedColor;
int numOfThreads = 100;

pthread_attr_t attr;
pthread_mutex_t mutexAddScc;
pthread_mutex_t mutexAddToSubg;
pthread_mutex_t mutexDeleteVertex;

void* trimStart(void* args){
    TrimArguments* arguments = (TrimArguments*) args;
    Graph* g = arguments->g;
    int vid = arguments->vid;

    //Check if the vertex with this ID is a start of an edge
    for(int startIndex=0;startIndex<g->startLength;startIndex++){
        if(g->start[startIndex] == vid){
            //Follow the edges and check if there is a self loop
            int ifinish = startIndex + 1 < g->startPointerLength ? g->startPointer[startIndex+1] : g->endLength;

            for(int endIndex=g->startPointer[startIndex];endIndex<ifinish;endIndex++){
                //If vertex has been removed
                int endvid = g->end[endIndex];
                if(endvid == -1){
                    continue;
                }

                //if there is an edge thats not a self loop
                if(endvid != vid){
                    arguments->timesFoundInStart++;
                    break;
                }
            }

            // timesFoundInStart++;
            // break;
        }
    }

    pthread_exit(NULL);
}

void* trimEnd(void* args){
    TrimArguments* arguments = (TrimArguments*) args;
    Graph* g = arguments->g;
    int vid = arguments->vid;

    for(int endIndex=0;endIndex<g->endLength;endIndex++){
        if(g->end[endIndex] == vid){
            arguments->timesFoundInEnd++;
            break;
        }
    }

    pthread_exit(NULL);
}

//Identifies and removes all trivial SCCs
void* parTrimGraph(void* args){
    //Graph* g, int startingVertex, int endingVertex
    Arguments* arguments = (Arguments*) args;
    Graph* g = arguments->g;
    int startingVertex = arguments->startIndex;
    int endingVertex = arguments->endIndex;
    //int threadId = arguments->id;

    // printf("Start of thread %d:%d\n", threadId, startingVertex);
    // printf("End of thread %d:%d\n", threadId, endingVertex);

    //For every vertex ID in vertices array of graph 
    for(int i=startingVertex;i<endingVertex;i++){
        int vid = g->vertices[i];
        TrimArguments* trimArgs = (TrimArguments*) malloc(sizeof(TrimArguments));
        trimArgs->vid = vid;
        trimArgs->g = arguments->g;
        trimArgs->timesFoundInStart = 0;
        trimArgs->timesFoundInEnd = 0;

        pthread_t trimThread[2];

        int rc;
        long j;

        //int timesFoundInStart = 0;
        rc = pthread_create(&trimThread[0], &attr, trimStart, (void*)trimArgs);

		if(rc){
			// printf("Error in thread #%ld! Return code from pthread_create() is %d\n", i, rc);
			exit(-1);
		}

        //Check if the vertex with this ID is an end of an edge
        //int timesFoundInEnd = 0;
        rc = pthread_create(&trimThread[1], &attr, trimEnd, (void*)trimArgs);

		if(rc){
			// printf("Error in thread #%ld! Return code from pthread_create() is %d\n", i, rc);
			exit(-1);
		}

        for(j=0;j<2;j++){
            rc = pthread_join(trimThread[j], NULL);

            if(rc){
                printf("ERROR; return code from pthread_join() is %d\n", rc);
                exit(-1);
            }
        }
        int timesFoundInStart = trimArgs->timesFoundInStart;
        int timesFoundInEnd = trimArgs->timesFoundInEnd;

        //If the in-degree or out-degree is zero trim the vertex
        if(timesFoundInEnd == 0 || timesFoundInStart == 0){
            // printf("Trimming vertex: %d\n", vid); 
            deleteIndexfromArray(g->vertices, i);
            pthread_mutex_lock(&mutexAddScc);
            parSccCounter++;
            g->numOfVertices--;
            pthread_mutex_unlock(&mutexAddScc);
        }
    }

    resizeArray(g->vertices, g->verticesLength);
    free(arguments);

    /* Arguments go to status */
	pthread_exit(NULL);
}

void createThreadsForTrim(Graph* g){
    pthread_t thread[numOfThreads];

    int rc;
	long i;
    int local;

    //Trim trvial SCCs to simplify the graph
    //Can be done in parallel

    local = g->verticesLength / numOfThreads + 1;
	for(i=0;i<numOfThreads;i++){
        // printf("In parallelScc: Creating thread #%ld\n", i);

        Arguments* args = (Arguments*) malloc(sizeof(Arguments));
        args->g = g;
        args->id = i;

        args->startIndex = i * local;
        args->endIndex = ((i + 1) * local) > g->verticesLength ? g->verticesLength : (i + 1) * local;

        rc = pthread_create(&thread[i], &attr, parTrimGraph, (void*)args);

		if(rc){
			// printf("Error in thread #%ld! Return code from pthread_create() is %d\n", i, rc);
			exit(-1);
		}
    }
    /* Free attribute and wait for the other threads */
	//pthread_attr_destroy(&attr);

	for(i=0;i<numOfThreads;i++){
		rc = pthread_join(thread[i], NULL);

		if(rc){
			printf("ERROR; return code from pthread_join() is %d\n", rc);
			exit(-1);
		}
		// printf("Main: completed join with thread %ld\n", i);
	}
}

//Initializes each vertex with a color which equals to its ID
void* parInitColor(void* args){
    //Graph* g, int* vertexColor, int startingVertex, int endingVertex
    Arguments* arguments = (Arguments*) args;
    Graph* g = arguments->g;
    int* vertexColor = arguments->vertexColor;
    int startingVertex = arguments->startIndex;
    int endingVertex = arguments->endIndex;
    //int threadId = arguments->id;

    // printf("Start of thread %d:%d\n", threadId, startingVertex);
    // printf("End of thread %d:%d\n", threadId, endingVertex);

    for(int i=startingVertex;i<endingVertex;i++){
        vertexColor[i] = -1;

        int vid = g->vertices[i];
        if(vid != -1)
            vertexColor[i] = vid;
    }   

    free(arguments);

    pthread_exit(NULL); 
}

void createThreadsForInitColor(Graph* g, int* vertexColor){
    pthread_t thread[numOfThreads];

    int rc;
	long i;
    int local;

    local = g->verticesLength / numOfThreads + 1;
    for(i=0;i<numOfThreads;i++){
        // printf("In parallelScc: Creating thread #%ld\n", i);

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
    //pthread_attr_destroy(&attr);

    for(i=0;i<numOfThreads;i++){
        rc = pthread_join(thread[i], NULL);
        if(rc){
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
        // printf("Main: completed join with thread %ld\n", i);
    }
}

//Spreads color forward following the path of the edges
void* parSpreadColor(void* args){
    //Graph* g, int* vertexColor, int startingVertex, int endingVertex
    Arguments* arguments = (Arguments*) args;
    Graph* g = arguments->g;
    int* vertexColor = arguments->vertexColor;
    int startingVertex = arguments->startIndex;
    int endingVertex = arguments->endIndex;
    //int threadId = arguments->id;

    // printf("Start of thread %d:%d\n", threadId, startingVertex);
    // printf("End of thread %d:%d\n", threadId, endingVertex);

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
                        vertexColor[i] = vertexColor[nextColorIndex];
                        parChangedColor = true;
                    }
                }
            }
        } 
    }
    free(arguments);

    pthread_exit(NULL);
}

void createThreadsForSpreadColor(Graph* g, int* vertexColor){
    pthread_t thread[numOfThreads];

    int rc;
	long i;
    int local;
    
    local = g->verticesLength / numOfThreads + 1;
    for(i=0;i<numOfThreads;i++){
        // printf("In parallelScc: Creating thread #%ld\n", i);

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
    //pthread_attr_destroy(&attr);

    for(i=0;i<numOfThreads;i++){
        rc = pthread_join(thread[i], NULL);
        if(rc){
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
        // printf("Main: completed join with thread %ld\n", i);
    }
}

// typedef struct SubgArgs{
//     Graph* g;
//     Graph* subg;
//     int* vc;
//     int vcLength;
//     int startIndex;
//     int endIndex;
//     int id;
// }SubgArgs;

// void* addToSubgraph(void* args){
//     SubgArgs* arguments = (SubgArgs*) args;
//     Graph* g = arguments->g;
//     Graph* subg = arguments->subg;
//     int* vc = arguments->vc;
//     int vcLength = arguments->vcLength;

//     int startingVertex = arguments->startIndex;
//     int endingVertex = arguments->endIndex;

//     //int threadId = arguments->id;

//     // printf("Start of thread %d:%d\n", threadId, startingVertex);
//     // printf("End of thread %d:%d\n", threadId, endingVertex);

//     //For every vertex in start find if its in vc
//     for(int startIndex=startingVertex;startIndex<endingVertex;startIndex++){
//         bool startInSubgraph = false;
//         int startid = g->start[startIndex];

//         //if startid is in vc
//         if(!notInArray(vc, vcLength, startid)){
//             //If start is in vc follow its edges
//             int ifinish = startIndex + 1 < g->startPointerLength ? g->startPointer[startIndex+1] : g->endLength;

//             for(int endIndex=g->startPointer[startIndex];endIndex<ifinish;endIndex++){
//                 int endid = g->end[endIndex];

//                 pthread_mutex_lock(&mutexAddToSubg);
//                 //if both vertices are in vc put them in the subgraph
//                 if(!notInArray(vc, vcLength, endid)){
                    
//                     if(!startInSubgraph){
//                         subg->start[subg->startLength++] = startid;
//                         startInSubgraph = true;

//                         subg->startPointer[subg->startPointerLength++] = subg->endLength;

//                         if(notInArray(subg->vertices, subg->verticesLength, startid))
//                             subg->vertices[subg->verticesLength++] = startid;
//                     }
//                     subg->end[subg->endLength++] = endid;

//                     if(notInArray(subg->vertices, subg->verticesLength, endid))
//                         subg->vertices[subg->verticesLength++] = endid;

                    
//                 }
//                 pthread_mutex_unlock(&mutexAddToSubg);
//             }
//         }
//     }

//     pthread_exit(NULL);
// }

// Graph* parCreateSubgraph(Graph* g, int* vc, int vcLength){
//     //Init subgraph
//     Graph* subg = (Graph*) malloc(sizeof(Graph));
//     subg->vertices = (int*) malloc(vcLength * sizeof(int));
//     subg->verticesLength = 0;

//     subg->end = (int*) malloc(g->endLength * sizeof(int));
//     subg->endLength = 0;

//     subg->start = (int*) malloc(g->startLength * sizeof(int));
//     subg->startLength = 0;

//     subg->startPointer = (int*) malloc(g->startPointerLength * sizeof(int));
//     subg->startPointerLength = 0;

//     int numOfSubgThreads = 4;
//     pthread_t thread[numOfSubgThreads];

//     int rc;
// 	long i;
//     int local;
    
//     //printf("StartLength=%d\n", g->startLength);
//     local = g->startLength / numOfSubgThreads + 1;
//     for(i=0;i<numOfSubgThreads;i++){
//         SubgArgs* args = (SubgArgs*) malloc(sizeof(SubgArgs));
//         args->subg = subg;
//         args->g = g;
//         args->vc = vc;
//         args->vcLength = vcLength;
//         args->startIndex = i * local;
//         args->endIndex = ((i + 1) * local) > g->startLength ? g->startLength : (i + 1) * local;
//         args->id = i;

//         rc = pthread_create(&thread[i], &attr, addToSubgraph, (void*)args);

//         if(rc){
//             printf("Error in thread #%ld! Return code from pthread_create() is %d\n", i, rc);
//             exit(-1);
//         }
//     }

//     for(i=0;i<numOfSubgThreads;i++){
//         rc = pthread_join(thread[i], NULL);
//         if(rc){
//             printf("ERROR; return code from pthread_join() is %d\n", rc);
//             exit(-1);
//         }
//         // printf("Main: completed join with thread %ld\n", i);
//     }
    
//     subg->numOfVertices = subg->verticesLength;

//     //Resize arrays to their final size
//     resizeArray(subg->end, subg->endLength);
//     resizeArray(subg->start, subg->startLength);
//     resizeArray(subg->startPointer, subg->startPointerLength);

//     // printf("Subgraph:\n");
//     // printGraph(subg);

//     return subg;
// }

// //Finds the number of in a subgraph of vertices with the same color
// Array* parFindSccOfColor(Graph* g, int* vertexColor, int color){
//     //Initialize an array vc for the vertices
//     int* vc = (int*) malloc(g->verticesLength * sizeof(int));
//     int vcLength = 0;

//     //Append in vc all vertices with the current color
//     for(int i=0;i<g->verticesLength;i++){
//         if(vertexColor[i] == color){
//             vc[vcLength++] = g->vertices[i];
//         }
//     }
//     resizeArray(vc, vcLength);

//     //If there is only one vertex with that color return the vertex
//     if(vcLength == 1){
//         Array* scc = (Array*) malloc(sizeof(Array));
//         scc->arr = vc;
//         scc->length = vcLength;
//         return scc;
//     }

//     // printf("VC: ");
//     // printArray(vc, vcLength);

//     //Create a subgraph with the vertices contained in vc
//     Graph* subg = parCreateSubgraph(g, vc, vcLength);

//     //Follow edges from sugraph with bfs and find the SCCs
//     Array* scc = bfs(subg, color);
//     // printf("SCC: ");
//     // printArray(scc->arr, scc->length);

//     free(vc);
//     free(subg);

//     return scc;
// }

void* parAccessUniqueColors(void* args){
    //Graph* g, Array* uc, int* vertexColor, int startingColor, int endingColor

    Arguments* arguments = (Arguments*) args;
    Graph* g = arguments->g;
    int* vertexColor = arguments->vertexColor;
    Array* uc = arguments->uc;
    int startingVertex = arguments->startIndex;
    int endingVertex = arguments->endIndex;

    for(int i=startingVertex;i<endingVertex;i++){
        int color = uc->arr[i];

        //Find all vertexes with color and put them in vc
        Array* scc = findSccOfColor(g, vertexColor, color);

        // printf("SccLength=%d", scc->length);
        //Count SCCs found and delete from graph all vertices contained in a SCC
        if(scc->length > 0){
            //MUTEX
            pthread_mutex_lock(&mutexAddScc);
            parSccCounter++;
            pthread_mutex_unlock(&mutexAddScc);

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
    free(arguments);

    pthread_exit(NULL);
}

void createThreadsForUniqueColor(Graph* g, Array* uc, int* vertexColor){
    pthread_t thread[numOfThreads];

    int rc;
	long i;
    int local;

    local = uc->length / numOfThreads + 1;
    //printf("Local=%d\n", local);
    for(i=0;i<numOfThreads;i++){
        // printf("In parallelScc: Creating thread #%ld\n", i);

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

    for(i=0;i<numOfThreads;i++){
        rc = pthread_join(thread[i], NULL);
        if(rc){
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
        // printf("Main: completed join with thread %ld\n", i);
    }
}

int parallelColorScc(Graph* g){
    //Init threads
    parSccCounter = 0;
 
    printf("NumOfVertices=%d\n", g->numOfVertices);

    //Initialize mutexes
    pthread_mutex_init(&mutexAddScc, NULL);
    pthread_mutex_init(&mutexAddToSubg, NULL);
    pthread_mutex_init(&mutexDeleteVertex, NULL);

    //Initialize and set thread detached attribute to joinable
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    printf("Trimming...\n");
    createThreadsForTrim(g);
    printf("Trimming ended\n");

    //Init VertexColor array
    //Each Index corresponds to de vertices array and the value is the color of the vertex
    int n = g->verticesLength;
    int* vertexColor = (int*) malloc(n * sizeof(int));

    while(g->numOfVertices > 0){
        if(g->numOfVertices == 1){
            parSccCounter++;
            break;
        }

        printf("Start\n");
        // printGraph(g);
        printf("NumOfVertices=%d\n", g->numOfVertices);

        //return parSccCounter;

        //Init each vertex color withe the vertex id
        //Can be done in Parallel
        printf("Initializing colors...\n");
        createThreadsForInitColor(g, vertexColor);
        printf("Colors initialized.\n");
        
        // printf("Vertex Color: ");
        // printArray(vertexColor, g->verticesLength);

        //Spread vertex color fw until there are no changes in vertexColor
        parChangedColor = true;
        printf("Spreading color...\n");
        while(parChangedColor){
            
            parChangedColor = false;
            createThreadsForSpreadColor(g, vertexColor);
        }    
        printf("Spreading color ended.\n");

        // printf("Vertex Color: ");
        // printArray(vertexColor, g->verticesLength);

        //Find all unique colors left in the vertexColor array
        //Can be done in parallel?
        Array* uc = findUniqueColors(vertexColor, n);

        printf("Number of Unique colors=%d\n", uc->length);

        printf("Finding scc number...\n");
        //For each unique color, do BFS for the for the subgraph with that color
        //Can be done in parallel
        createThreadsForUniqueColor(g, uc, vertexColor);
        
        //parAccessUniqueColors(g, uc, vertexColor, 0, uc->length);

        printf("NumOfVertices=%d\n", g->numOfVertices);
        printf("SCCs found=%d\n", parSccCounter);
        free(uc);
    }

    return parSccCounter;
    pthread_exit(NULL);
    pthread_mutex_destroy(&mutexAddScc);
    pthread_mutex_destroy(&mutexAddToSubg);
    pthread_mutex_destroy(&mutexDeleteVertex);
    pthread_attr_destroy(&attr);
}