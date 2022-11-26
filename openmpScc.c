#include "openmpScc.h"

//global vars
int sccCounter;
bool changedColor;

pthread_mutex_t mutex;
pthread_mutex_t mutexDelVertex;

//Returns a COO array from an mtx file
CooArray* readMtxFile(char* filename){
    int ret_code;
    MM_typecode matcode;
    FILE *f;
    fpos_t pos;
    int M, N, nz;   
    int i, *I, *J;
    double* val;

    char filepath[50];
    strcpy(filepath, "graphs/");
    strcat(filepath, filename);
    strcat(filepath, ".mtx");

    int numOfCols = 1;
    if ((f = fopen(filepath, "r")) == NULL){
        printf("File with name <%s> not found in graphs/", filename);
        exit(1);
    }

    if (mm_read_banner(f, &matcode) != 0){
        printf("Could not process Matrix Market banner.\n");
        exit(1);
    }

    /*  This is how one can screen matrix types if their application */
    /*  only supports a subset of the Matrix Market data types.      */

    if (mm_is_complex(matcode) && mm_is_matrix(matcode) && mm_is_sparse(matcode) ){
        printf("Sorry, this application does not support ");
        printf("Market Market type: [%s]\n", mm_typecode_to_str(matcode));
        exit(1);
    }

    /* find out size of sparse matrix .... */

    if ((ret_code = mm_read_mtx_crd_size(f, &M, &N, &nz)) !=0){
        exit(1);
    }

    fgetpos(f, &pos);
    char str[150];
    if(fgets(str,150, f) == NULL){
        printf("Error: Cannot read graph!\n");
    }

    i=0;
    while(i<=str[i]){
        if(str[i]==' '){
            numOfCols++;
        }
        i++;
    }
    fsetpos(f, &pos);

    /* reseve memory for matrices */
    I = (int*) malloc(nz * sizeof(int));
    J = (int*) malloc(nz * sizeof(int));
    val = (double *) malloc(nz * sizeof(double));


    /* NOTE: when reading in doubles, ANSI C requires the use of the "l"  */
    /*   specifier as in "%lg", "%lf", "%le", otherwise errors will occur */
    /*  (ANSI C X3.159-1989, Sec. 4.9.6.2, p. 136 lines 13-15)            */

    if(numOfCols == 3){
        //For weighted graphs
        for(i=0; i<nz; i++){
            if(fscanf(f, "%d %d %lg\n", &I[i], &J[i], &val[i]) == 0){
                printf("Error: Cannot read graph!\n");
            }
            I[i]--;  /* adjust from 1-based to 0-based */
            J[i]--;
        }

        if (f !=stdin) fclose(f);

        /************************/
        /* now write out matrix */
        /************************/

        mm_write_banner(stdout, matcode);
        mm_write_mtx_crd_size(stdout, M, N, nz);
        // for(i=0; i<nz; i++){
        //     fprintf(stdout, "%d %d %20.19g\n", I[i]+1, J[i]+1, val[i]);
        // }
    }
    else if(numOfCols == 2){
        //For unweighted graphs
        for(i=0; i<nz; i++){
            if(fscanf(f, "%d %d\n", &I[i], &J[i]) == 0){
                printf("Error: Cannot read graph!\n");
            }
            I[i]--;  /* adjust from 1-based to 0-based */
            J[i]--;
        }

        if (f !=stdin) fclose(f);

        mm_write_banner(stdout, matcode);
        mm_write_mtx_crd_size(stdout, M, N, nz);
        // for(i=0; i<nz; i++){
        //     fprintf(stdout, "%d %d\n", I[i]+1, J[i]+1);
        // }
    }
    else{
        printf("Error: Number of columns not 2 or 3!\n");
        exit(1);
    }

    CooArray* cooArray = (CooArray*) malloc(sizeof(CooArray));
    cooArray->i = I;
    cooArray->j = J;
    cooArray->iLength = nz;
    cooArray->jLength = nz;
    cooArray->numOfVertices = M;
    printf("\n");

	return cooArray;
}

//Identifies and removes all trivial SCCs
void trimGraph(Graph* g, int startingVertex, int endingVertex){
    //For every vertex ID in vertices array of graph
    int sccTrimCounter = 0;
    
    #pragma omp parallel for reduction(+:sccTrimCounter)
        for(int i=startingVertex;i<endingVertex;i++){

            //If the in-degree or out-degree is zero trim the vertex
            if(g->inDegree[i] == 0 || g->outDegree[i] == 0){
                //printf("Trimming vertex: %d\n", vid);
                deleteIndexfromArray(g->vertices, i);
                sccTrimCounter++;
            }
        }

    sccCounter += sccTrimCounter;
    g->numOfVertices -= sccTrimCounter;
    //resizeArray(g->vertices, g->verticesLength);
}

//Initializes graph from a given COO array
Graph* initGraphFromCoo(CooArray* ca){
    Graph* g = (Graph*) malloc(sizeof(Graph));
    g->end = ca->i;
    g->endLength = ca->iLength;

    //Malloc to size jLength because we dont know the final size
    g->start = (int*) malloc(ca->jLength * sizeof(int));
    g->startLength = 0;
    g->startPointer = (int*) malloc(ca->jLength * sizeof(int));
    g->startPointerLength = 0;

    g->vertices = (int*) malloc(ca->numOfVertices * sizeof(int));
    g->vertexPosInStart = (int*) malloc(ca->numOfVertices * sizeof(int));
    g->verticesLength = ca->numOfVertices;
    for(int i=0;i<g->verticesLength;i++){
        g->vertices[i] = i;
        g->vertexPosInStart[i] = -1;
    }

    g->inDegree = (int*) calloc(ca->numOfVertices, sizeof(int));
    g->outDegree = (int*) calloc(ca->numOfVertices, sizeof(int));

    //Find all vertices from the J (start) array
    int vid = -1;
    for(int index=0;index<ca->jLength;index++){
        if(vid != ca->j[index]){
            vid = ca->j[index];
            g->start[g->startLength] = vid;
            g->vertexPosInStart[vid] = g->startLength;
            g->startLength++;
            g->startPointer[g->startPointerLength] = index;
            g->startPointerLength++;

            g->outDegree[vid]++;
        }

        //int indexInVertices = getIndexOfValue(g->vertices, g->verticesLength, ca->j[index]);
        //g->outDegree[indexInVertices]++;
    }

    //Get all remaining vertices from I (end) array
    for(int i=0;i<g->endLength;i++){
        g->inDegree[g->end[i]]++;
    }

    g->numOfVertices = g->verticesLength;

    //mergeSort(g->vertices, 0, g->numOfVertices);

    //printArray(g->vertexPosInStart, g->numOfVertices);

    free(ca);
    return g;
}

//Initializes each vertex with a color which equals to its ID
void initColor(Graph* g, int* vertexColor, int startingVertex, int endingVertex){
    #pragma omp parallel for
        for(int i=startingVertex;i<endingVertex;i++){
            vertexColor[i] = -1;

            int vid = g->vertices[i];
            if(vid != -1)
                vertexColor[i] = vid;
        }
}

//Spreads color forward following the path of the edges
void spreadColor(Graph* g, int* vertexColor, int startingVertex, int endingVertex){
    //good for parallelism
    #pragma omp parallel for
        for(int i=startingVertex;i<endingVertex;i++){
            int vid = g->vertices[i];
            if(vid == -1){
                continue;
            } 

            int color = vertexColor[vid];
            if(color == 0)
                continue;
            //int vid = g->vertices[i];

            int startIndex = g->vertexPosInStart[vid];

            //Follow the edges and spread color to the end vertices
            int ifinish = startIndex + 1 < g->startPointerLength ? g->startPointer[startIndex+1] : g->endLength;

            for(int endIndex=g->startPointer[startIndex];endIndex<ifinish;endIndex++){
                //If vertex has been removed
                int endvid = g->vertices[g->end[endIndex]];
                if(endvid == -1){
                    continue;
                }

                int nextColor = vertexColor[endvid];

                if(nextColor < color){
                    vertexColor[vid] = vertexColor[endvid];
                    changedColor = true;
                }
            }
        }
}

//Returns all unique colors contained in vertexColor array
Array* findUniqueColors(int* vertexColor, int size){
    Array* uniqueColors = (Array*) malloc(sizeof(Array));
    uniqueColors->arr = (int*) malloc(size * sizeof(int));
    uniqueColors->length = 0;

    for(int i=0;i<size;i++){
        int color = vertexColor[i];
        if(color == -1){
            continue;
        }

        if(notInArray(uniqueColors->arr, uniqueColors->length, color)){
            uniqueColors->arr[uniqueColors->length++] = color;
        }
    }

    //resizeArray(uniqueColors->arr, uniqueColors->length);

    return uniqueColors;
}

void accessUniqueColors(Graph* g, Array* uc, int* vertexColor, int startingColor, int endingColor){
    int sccUcCounter = 0;
    int sccNumOfVertices = 0;

    #pragma omp parallel for reduction(+:sccUcCounter, sccNumOfVertices)
        for(int i=startingColor;i<endingColor;i++){
            // printf("Vertex Color: ");
            // printArray(vertexColor, g->verticesLength);

            int color = uc->arr[i];

            // printf("Color:%d\n", color);
            //Find all vertexes with color and put them in vc
            Array* scc = bfs(g, color, vertexColor);

            // printf("SccLength=%d", scc->length);
            //Count SCCs found and delete from graph all vertices contained in a SCC
            if(scc->length > 0){
                sccUcCounter++;
                sccNumOfVertices += scc->length;

                //Delete each vertex with if found in scc
                for(int j=0;j<scc->length;j++){
                    int vid = scc->arr[j];
                    deleteVertexFromGraph(g, vid);
                    // g->numOfVertices--;
                }
            }
            else{
                printf("Error: Did not find any SCCs for color=%d!\n", color);
                exit(1);
            }
            free(scc);
        }

    sccCounter += sccUcCounter;
    g->numOfVertices -= sccNumOfVertices;
}

void printArray(int* array, int n){
    for(int i=0;i<n;i++){
        printf("%d ", array[i]);
    }
    printf("\n");
}

int openmpColorScc(Graph* g, bool trimming){
    sccCounter = 0;

    //Initialize mutexes
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutexDelVertex, NULL);
    
    //Trim trvial SCCs to simplify the graph
    //Can be done in parallel
    if(trimming){
        printf("Trimming...\n");
        trimGraph(g, 0, g->verticesLength);
        printf("Trimming ended\n");
    }
    
    //Init VertexColor array
    //Each Index corresponds to de vertices array and the value is the color of the vertex
    int n = g->verticesLength;
    int* vertexColor = (int*) malloc(n * sizeof(int));

    while(g->numOfVertices > 0){
        if(g->numOfVertices == 1){
            sccCounter++;
            break;
        }

        printf("Start\n");
        // printGraph(g);
        printf("NumOfVertices=%d\n", g->numOfVertices);

        //Init each vertex color withe the vertex id
        //Can be done in Parallel
        initColor(g, vertexColor, 0, n);
        // printf("Vertex Color: ");

        //Spread vertex color fw until there are no changes in vertexColor
        changedColor = true;
        printf("Spreading color...\n");
        while(changedColor){     
            changedColor = false;
            
            //Can be done in Parallel
            spreadColor(g, vertexColor, 0, n);
        }
        
        printf("Spreading color ended\n");

        // printf("Vertex Color: ");

        //Find all unique colors left in the vertexColor array
        Array* uc = findUniqueColors(vertexColor, n);

        printf("Number of Unique colors=%d\n", uc->length);

        printf("Finding scc number...\n");
        //For each unique color, do BFS for the for the subgraph with that color
        //Can be done in parallel
        accessUniqueColors(g, uc, vertexColor, 0, uc->length);

        printf("NumOfVertices=%d\n", g->numOfVertices);
        printf("SCCs found=%d\n", sccCounter);
        free(uc);
    }

    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutexDelVertex);
    return sccCounter;
}