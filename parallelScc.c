#include "seqScc.h"

//global vars
int parSccCounter;
bool parChangedColor;

//Identifies and removes all trivial SCCs
void parTrimGraph(Graph* g, int startingVertex, int endingVertex){
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
            printf("Trimming vertex: %d\n", vid);
            deleteIndexfromArray(g->vertices, i);
            parSccCounter++;
            g->numOfVertices--;
        }
    }

    resizeArray(g->vertices, g->verticesLength);
}

//Initializes each vertex with a color which equals to its ID
void parInitColor(Graph* g, int* vertexColor, int startingVertex, int endingVertex){
    for(int i=startingVertex;i<endingVertex;i++){
        vertexColor[i] = -1;

        int vid = g->vertices[i];
        if(vid != -1)
            vertexColor[i] = vid;
    }    
}

//Spreads color forward following the path of the edges
void parSpreadColor(Graph* g, int* vertexColor, int startingVertex, int endingVertex){
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
}

void parAccessUniqueColors(Graph* g, Array* uc, int* vertexColor, int startingColor, int endingColor){
    for(int i=0;i<uc->length;i++){
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
                deleteVertexFromGraph(g, vertexColor, vid);
            }
        }
        else{
            printf("Error: Did not find any SCCs for color=%d!\n", color);
            exit(1);
        }
        free(scc);
    }
}

int numOfThreads = 10;

typedef struct Arguments{
    Graph* g;
    Array* uc;
    int* vertexColor;
}Arguments;

int parallelColorScc(Graph* g){
    //Init threads
    //pthread_t thread[numOfThreads];

    parSccCounter = 0;
    printf("Trimming...\n");
    //Trim trvial SCCs to simplify the graph
    //Can be done in parallel
    // int rc;
	// long i;

	// for(i=0;i<numOfThreads;i++){
    //     printf("In parallelScc: Creating thread #%ld\n", i);

    //     rc = pthread_create(&thread[i], NULL, trimGraph, (void*)i);

	// 	if(rc){
	// 		printf("Error in thread #%ld! Return code from pthread_create() is %d\n", i, rc);
	// 		exit(-1);
	// 	}
    // }
    parTrimGraph(g, 0, g->verticesLength);
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

        //Init each vertex color withe the vertex id
        //Can be done in Parallel
        parInitColor(g, vertexColor, 0, n);
        // printf("Vertex Color: ");
        // printArray(vertexColor, g->verticesLength);

        //Spread vertex color fw until there are no changes in vertexColor
        parChangedColor = true;
        while(parChangedColor){
            printf("Spreading color...\n");
            parChangedColor = false;
            
            //Can be done in Parallel
            parSpreadColor(g, vertexColor, 0, n);
        }
        
        printf("Spreading color ended\n");

        // printf("Vertex Color: ");
        // printArray(vertexColor, g->verticesLength);

        //Find all unique colors left in the vertexColor array
        Array* uc = findUniqueColors(vertexColor, n);

        printf("Number of Unique colors=%d\n", uc->length);

        printf("Finding scc number...\n");
        //For each unique color, do BFS for the for the subgraph with that color
        //Can be done in parallel
        parAccessUniqueColors(g, uc, vertexColor, 0, uc->length);

        printf("NumOfVertices=%d\n", g->numOfVertices);
        printf("SCCs found=%d\n", parSccCounter);
        free(uc);
    }

    //pthread_exit(NULL);
    return parSccCounter;
}