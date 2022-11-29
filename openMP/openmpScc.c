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
    strcpy(filepath, "../graphs/");
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

    free(val);

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
    free(ca->j);
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

void merge(int arr[], int l, int m, int r)
{
	int i, j, k;
	int n1 = m - l + 1;
	int n2 = r - m;

	/* create temp arrays */
	// int L[n1], R[n2];
    int* L = (int*) malloc(n1 * sizeof(int));
    int* R = (int*) malloc(n2 * sizeof(int));

	/* Copy data to temp arrays L[] and R[] */
	for (i = 0; i < n1; i++)
		L[i] = arr[l + i];
	for (j = 0; j < n2; j++)
		R[j] = arr[m + 1 + j];

	/* Merge the temp arrays back into arr[l..r]*/
	i = 0; // Initial index of first subarray
	j = 0; // Initial index of second subarray
	k = l; // Initial index of merged subarray
	while (i < n1 && j < n2) {
		if (L[i] <= R[j]) {
			arr[k] = L[i];
			i++;
		}
		else {
			arr[k] = R[j];
			j++;
		}
		k++;
	}

	/* Copy the remaining elements of L[], if there
	are any */
	while (i < n1) {
		arr[k] = L[i];
		i++;
		k++;
	}

	/* Copy the remaining elements of R[], if there
	are any */
	while (j < n2) {
		arr[k] = R[j];
		j++;
		k++;
	}

    free(L);
    free(R);
}

/* l is for left index and r is right index of the
sub-array of arr to be sorted */
void mergeSort(int arr[], int l, int r)
{
	if (l < r) {
		// Same as (l+r)/2, but avoids overflow for
		// large l and h
		int m = l + (r - l) / 2;

		// Sort first and second halves
		mergeSort(arr, l, m);
		mergeSort(arr, m + 1, r);

		merge(arr, l, m, r);
	}
}

int* copyArray(int const* src, int len)
{
   int* p = malloc(len * sizeof(int));
   if(p == NULL)
    printf("Error: malloc failed in copy array\n");
   memcpy(p, src, len * sizeof(int));
   return p;
}

//Returns all unique colors contained in vertexColor array
Array* findUniqueColors(int* vertexColor, int size){
    Array* uniqueColors = (Array*) malloc(sizeof(Array));
    uniqueColors->arr = (int*) malloc(size * sizeof(int));
    uniqueColors->length = 0;

    int* temp = copyArray(vertexColor, size);

    //printArray(temp, size);
    mergeSort(temp, 0, size - 1);

    //printArray(temp, size);

    for(int i=0;i<size;i++){
        int color = temp[i];
        if(color == -1){
            continue;
        }

        while(i < size - 1 && temp[i] == temp[i+1])
            i++;

        uniqueColors->arr[uniqueColors->length++] = color;
    }

    free(temp);

    return uniqueColors;
}

void accessUniqueColors(Graph* g, Array* uc, int* vertexColor, int startingColor, int endingColor){
    int sccUcCounter = 0;
    int sccNumOfVertices = 0;

    int n = g->verticesLength;

    Queue* queueArr[endingColor - startingColor];
    Array* sccArr[endingColor - startingColor];

    #pragma omp parallel for reduction(+:sccUcCounter, sccNumOfVertices)
        for(int i=startingColor;i<endingColor;i++){
            // printf("Vertex Color: ");
            // printArray(vertexColor, g->verticesLength);

            int color = uc->arr[i];

            Queue* queue = queueArr[i];
            queue = (Queue*) malloc(sizeof(Queue));
            queueInit(queue, n);
            Array* scc = sccArr[i];
            scc = (Array*) malloc(sizeof(Array));
            scc->arr = (int*) malloc(n * sizeof(int));
            scc->length = 0;

            // printf("Color:%d\n", color);
            //Find all vertexes with color and put them in vc
            bfs(g, color, vertexColor, queue, scc);

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

            free(queue->arr);
            free(queue);
            free(scc->arr);
            free(scc);
        }

    sccCounter += sccUcCounter;
    g->numOfVertices -= sccNumOfVertices;
}

int sumOfArray(int* array, int size){
    int sum = 0;
    for(int i=0;i<size;i++){
        sum += array[i];
    }

    return sum;
}

// void accessUniqueColors(Graph* g, Array* uc, int* vertexColor, int startingColor, int endingColor){
//     int sccUcCounter[endingColor - startingColor];
//     int sccNumOfVertices[endingColor - startingColor];

//     int n = g->verticesLength;

//     Queue* queueArr[endingColor - startingColor];
//     Array* sccArr[endingColor - startingColor];

//     #pragma omp parallel for
//     for(int i=startingColor;i<endingColor;i++){
//         // printf("Vertex Color: ");
//         // printArray(vertexColor, g->verticesLength);

//         sccUcCounter[i] = 0;
//         sccNumOfVertices[i] = 0;

//         int color = uc->arr[i];

//         Queue* queue = queueArr[i];
//         queue = (Queue*) malloc(sizeof(Queue));
//         queueInit(queue, n);
//         Array* scc = sccArr[i];
//         scc = (Array*) malloc(sizeof(Array));
//         scc->arr = (int*) malloc(n * sizeof(int));
//         scc->length = 0;

//         // printf("Color:%d\n", color);
//         //Find all vertexes with color and put them in vc
//         bfs(g, color, vertexColor, queue, scc);

//         // printf("SccLength=%d", scc->length);
//         //Count SCCs found and delete from graph all vertices contained in a SCC
//         if(scc->length > 0){
//             sccUcCounter[i] = 1;
//             sccNumOfVertices[i] = scc->length;

//             //Delete each vertex with if found in scc
//             for(int j=0;j<scc->length;j++){
//                 int vid = scc->arr[j];
//                 deleteVertexFromGraph(g, vid);
//                 // g->numOfVertices--;
//             }
//         }
//         else{
//             printf("Error: Did not find any SCCs for color=%d!\n", color);
//             exit(1);
//         }

//         free(queue->arr);
//         free(queue);
//         free(scc->arr);
//         free(scc);
//     }

//     int sumSccUcCounter = 0;
//     int sumSccNumOfVertices = 0;

//     #pragma omp parallel
//         sumSccUcCounter = sumOfArray(sccUcCounter, endingColor - startingColor);
//         sumSccNumOfVertices = sumOfArray(sccNumOfVertices, endingColor - startingColor);
    
//     sccCounter += sumSccUcCounter;
//     g->numOfVertices -= sumSccNumOfVertices;
// }

void printArray(int* array, int n){
    for(int i=0;i<n;i++){
        printf("%d ", array[i]);
    }
    printf("\n");
}

int openmpColorScc(Graph* g, bool trimming){
    struct timeval startwtime, endwtime;
    double duration;
    sccCounter = 0;

    //Initialize mutexes
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutexDelVertex, NULL);
    
    //Trim trvial SCCs to simplify the graph
    //Can be done in parallel
    if(trimming){
        printf("Trimming...\n");
        gettimeofday (&startwtime, NULL);
        trimGraph(g, 0, g->verticesLength);
        gettimeofday (&endwtime, NULL);
        duration = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);
        printf("Trimming ended in %.4f seconds\n", duration);
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
        printf("Initializing colors...\n");
        initColor(g, vertexColor, 0, n);
        printf("Colors initialized.\n");
        // printf("Vertex Color: ");

        //Spread vertex color fw until there are no changes in vertexColor
        changedColor = true;
        printf("Spreading color...\n");
        gettimeofday (&startwtime, NULL);
        while(changedColor){     
            changedColor = false;
            
            //Can be done in Parallel
            spreadColor(g, vertexColor, 0, n);
        }
        
        gettimeofday (&endwtime, NULL);
        duration = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);
        printf("Spreading color ended in %.4f seconds\n", duration);

        // printf("Vertex Color: ");

        //Find all unique colors left in the vertexColor array
        printf("Finding unique colors...\n");
        gettimeofday (&startwtime, NULL);
        Array* uc = findUniqueColors(vertexColor, n);

        gettimeofday (&endwtime, NULL);
        duration = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);
        printf("Number of Unique colors=%d, found in %.4f seconds\n", uc->length, duration);

        printf("Finding scc number...\n");
        //For each unique color, do BFS for the for the subgraph with that color
        //Can be done in parallel
        gettimeofday (&startwtime, NULL);
        accessUniqueColors(g, uc, vertexColor, 0, uc->length);

        gettimeofday (&endwtime, NULL);
        duration = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);

        printf("NumOfVertices=%d\n", g->numOfVertices);
        printf("SCCs found=%d in %.4f seconds\n", sccCounter, duration);

        free(uc->arr);
        free(uc);
    }

    free(vertexColor);

    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutexDelVertex);
    return sccCounter;
}