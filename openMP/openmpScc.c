#include "openmpScc.h"

//global vars
int sccCounter;
bool changedColor;

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

        mm_write_banner(stdout, matcode);
        mm_write_mtx_crd_size(stdout, M, N, nz);
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

//Calculates degrees of each vertex
void calculateVertexDegrees(Graph* g){
    //Find all vertices from the J (start) array
    #pragma omp parallel for
        for(int i=0;i<g->endLength;i++){
            int startId = g->startAll[i];
            int endId = g->end[i];

            if(g->vertices[startId] == -1 || g->vertices[endId] == -1) continue;
            //self loops
            if(g->vertices[startId] == g->vertices[endId]) continue;

            g->outDegree[startId]++;
            g->inDegree[endId]++;
        }
}

//Identifies and removes all trivial SCCs
void trimGraph(Graph* g, int startingVertex, int endingVertex){
    calculateVertexDegrees(g);
    //For every vertex ID in vertices array of graph
    int sccTrimCounter = 0;
    
    #pragma omp parallel for reduction(+:sccTrimCounter)
        for(int i=startingVertex;i<endingVertex;i++){
            if(g->vertices[i] == -1) continue;

            //If the in-degree or out-degree is zero trim the vertex
            if(g->inDegree[i] == 0 || g->outDegree[i] == 0){
                deleteIndexfromArray(g->vertices, i);
                g->sccIdOfVertex[i] = sccCounter + sccTrimCounter++;
            }

            g->inDegree[i] = 0;
            g->outDegree[i] = 0;
        }

    sccCounter += sccTrimCounter;
    g->numOfVertices -= sccTrimCounter;
}

//Initializes graph from a given COO array
Graph* initGraphFromCoo(CooArray* ca){
    Graph* g = (Graph*) malloc(sizeof(Graph));
    g->end = ca->i;
    g->endLength = ca->iLength;

    g->startAll = ca->j;
    g->sccIdOfVertex = (int*) malloc(ca->numOfVertices * sizeof(int));

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

    int vid = -1;
    for(int index=0;index<ca->jLength;index++){
        if(vid != ca->j[index]){
            vid = ca->j[index];
            g->start[g->startLength] = vid;
            g->vertexPosInStart[vid] = g->startLength;
            g->startLength++;
            g->startPointer[g->startPointerLength] = index;
            g->startPointerLength++;
        }
    }

    g->numOfVertices = g->verticesLength;

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
    #pragma omp parallel for 
        for(int i=startingVertex;i<endingVertex;i++){
            int vid = g->vertices[i];
            if(vid == -1){
                continue;
            } 

            int color = vertexColor[vid];
            if(color == 0)
                continue;

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

void merge(int arr[], int l, int m, int r){
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
void mergeSort(int arr[], int l, int r){
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

int* copyArray(int const* src, int len){
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

    mergeSort(temp, 0, size - 1);

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

//Finds the SCC for every unique color
void accessUniqueColors(Graph* g, Array* uc, int* vertexColor, int startingColor, int endingColor){
    int sccUcCounter = 0;
    int sccNumOfVertices = 0;

    int n = g->verticesLength;

    Queue* queueArr[endingColor - startingColor];
    Array* sccArr[endingColor - startingColor];

    #pragma omp parallel for reduction(+:sccUcCounter, sccNumOfVertices)
        for(int i=startingColor;i<endingColor;i++){
            int color = uc->arr[i];

            Queue* queue = queueArr[i];
            queue = (Queue*) malloc(sizeof(Queue));
            queueInit(queue, n);
            Array* scc = sccArr[i];
            scc = (Array*) malloc(sizeof(Array));
            scc->arr = (int*) malloc(n * sizeof(int));
            scc->length = 0;

            //Find all vertexes with color and put them in vc
            bfs(g, color, vertexColor, queue, scc);

            //Count SCCs found and delete from graph all vertices contained in a SCC
            if(scc->length > 0){
                sccUcCounter++;
                sccNumOfVertices += scc->length;

                //Delete each vertex with if found in scc
                for(int j=0;j<scc->length;j++){
                    int vid = scc->arr[j];
                    g->sccIdOfVertex[vid] = sccCounter + sccUcCounter - 1;
                    deleteVertexFromGraph(g, vid);
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

int openmpColorScc(Graph* g, bool trimming){
    sccCounter = 0;
      
    //Init VertexColor array
    //Each Index corresponds to de vertices array and the value is the color of the vertex
    int n = g->verticesLength;
    int* vertexColor = (int*) malloc(n * sizeof(int));

    while(g->numOfVertices > 0){

        //Trim trvial SCCs to simplify the graph
        if(trimming){
            trimGraph(g, 0, g->verticesLength);
        }

        //Init each vertex color withe the vertex id
        initColor(g, vertexColor, 0, n);

        //Spread vertex color fw until there are no changes in vertexColor
        changedColor = true;
        while(changedColor){     
            changedColor = false;

            spreadColor(g, vertexColor, 0, n);
        }

        //Find all unique colors left in the vertexColor array
        Array* uc = findUniqueColors(vertexColor, n);

        //For each unique color, do BFS for the for the subgraph with that color
        accessUniqueColors(g, uc, vertexColor, 0, uc->length);

        free(uc->arr);
        free(uc);
    }
    free(vertexColor);

    return sccCounter;
}