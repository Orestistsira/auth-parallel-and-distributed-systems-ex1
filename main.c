#include <string.h>
#include <time.h>

#include "bfs.h"
#include "mmio.h"

typedef struct CooArray{
    int* i;
    int* j;

    int iLength;
    int jLength;
    int numOfVertices;
}CooArray;

void printArray(int* array, int n){
    for(int i=0;i<n;i++){
        printf("%d ", array[i]);
    }
    printf("\n");
}

void printGraph(Graph* g){
    printf("End:---------------------------------------------------------------------------------------\n");
    printArray(g->end, g->endLength);
    printf("Start:-------------------------------------------------------------------------------------\n");
    printArray(g->start, g->startLength);
    printf("Start Pointer------------------------------------------------------------------------------\n");
    printArray(g->startPointer, g->startPointerLength);
    printf("Vertices-----------------------------------------------------------------------------------\n");
    printArray(g->vertices, g->numOfVertices);
    printf("NoV=%d\n", g->numOfVertices);
    printf("\n");
}

// void resizeArray(int* arr, int newSize){
//     int* temp = (int*) realloc(arr, newSize * sizeof(int));
//     arr = temp;

// }

int* copyArray(int* arr, int size){
    int* temp = (int*) malloc(size * sizeof(int));
    if(temp != NULL)
        memcpy(temp, arr, size * sizeof(int));
    return temp;
}

bool notInArray(int* arr, int size, int value){
    for(int i=0;i<size;i++){
        if(arr[i] == value)
            return false;
    }
    return true;
}

void bfsTest(Graph* g, int source){
    Array* sccList = bfs(g, source);

    printf("Result: ");
    printArray(sccList->arr, sccList->length);

    free(sccList);
}

CooArray* readMtxFile(char* file){
    int ret_code;
    MM_typecode matcode;
    FILE *f;
    int M, N, nz;   
    int i, *I, *J;
    double* val;

    if ((f = fopen(file, "r")) == NULL){
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

    /* reseve memory for matrices */

    I = (int*) malloc(nz * sizeof(int));
    J = (int*) malloc(nz * sizeof(int));
    val = (double *) malloc(nz * sizeof(double));


    /* NOTE: when reading in doubles, ANSI C requires the use of the "l"  */
    /*   specifier as in "%lg", "%lf", "%le", otherwise errors will occur */
    /*  (ANSI C X3.159-1989, Sec. 4.9.6.2, p. 136 lines 13-15)            */

    for(i=0; i<nz; i++){
        fscanf(f, "%d %d %lg\n", &I[i], &J[i], &val[i]);
        I[i]--;  /* adjust from 1-based to 0-based */
        J[i]--;
    }

    // for(i=0; i<nz; i++){
    //     fscanf(f, "%d %d\n", &I[i], &J[i]);
    //     I[i]--;  /* adjust from 1-based to 0-based */
    //     J[i]--;
    // }

    if (f !=stdin) fclose(f);

    /************************/
    /* now write out matrix */
    /************************/

    mm_write_banner(stdout, matcode);
    mm_write_mtx_crd_size(stdout, M, N, nz);
    for(i=0; i<nz; i++){
        fprintf(stdout, "%d %d %20.19g\n", I[i]+1, J[i]+1, val[i]);
    }

    // for(i=0; i<nz; i++){
    //     fprintf(stdout, "%d %d\n", I[i]+1, J[i]+1);
    // }

    CooArray* cooArray = (CooArray*) malloc(sizeof(CooArray));
    cooArray->i = I;
    cooArray->j = J;
    cooArray->iLength = nz;
    cooArray->jLength = nz;
    cooArray->numOfVertices = M;

	return cooArray;
}

int trimGraph(Graph* g){
    int sccCounter = 0;

    for(int i=0;i<g->numOfVertices;i++){
        int vid = g->vertices[i];

        int timesFoundInStart = 0;
        for(int startIndex=0;startIndex<g->startLength;startIndex++){
            if(g->start[startIndex] == vid){
                timesFoundInStart++;
                break;
            }
        }

        int timesFoundInEnd = 0;
        for(int endIndex=0;endIndex<g->startLength;endIndex++){
            if(g->start[endIndex] == vid){
                timesFoundInEnd++;
                break;
            }
        }

        if(timesFoundInEnd == 0 || timesFoundInStart == 0){
            printf("Trimming vertex: %d\n", vid);
            //deleteVertexFromGraph(g, g->vertices, vid);
            deleteIndexfromArray(g->vertices, g->numOfVertices, i);
            sccCounter++;
            g->numOfVertices--;
            i--;
        }
    }

    resizeArray(g->vertices, g->numOfVertices);

    return sccCounter;
}

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
    g->numOfVertices = 0;

    int vid = -1;
    for(int index=0;index<ca->jLength;index++){
        if(vid != ca->j[index]){
            vid = ca->j[index];
            g->start[g->startLength] = vid;
            g->startLength++;
            g->startPointer[g->startPointerLength] = index;
            g->startPointerLength++;

            g->vertices[g->numOfVertices] = vid;
            g->numOfVertices++;
        }
    }

    //Get all remaining vertices
    if(g->numOfVertices != ca->numOfVertices){
        for(int i=0;i<g->endLength;i++){
            if(notInArray(g->vertices, g->numOfVertices, g->end[i])){
                g->vertices[g->numOfVertices] = g->end[i];
                g->numOfVertices++;
            }
        }
    }

    free(ca);

    //Resize arrays to their length
    resizeArray(g->start, g->startLength);
    resizeArray(g->startPointer, g->startPointerLength);

    return g;
}

void initColor(int* vertexColor, int index, int color){
    vertexColor[index] = color;
}

void spreadColor(){
    
}

Array* findUniqueColors(int* vertexColor, int size){
    Array* uniqueColors = (Array*) malloc(sizeof(Array));
    uniqueColors->arr = (int*) malloc(size * sizeof(int));
    uniqueColors->length = 0;

    for(int i=0;i<size;i++){
        int color = vertexColor[i];
        if(notInArray(uniqueColors->arr, uniqueColors->length, color)){
            uniqueColors->arr[uniqueColors->length++] = color;
        }
    }

    resizeArray(uniqueColors->arr, uniqueColors->length);

    return uniqueColors;
}

Graph* createSubgraph(Graph* g, int* vc, int vcLength){
    Graph* subg = (Graph*) malloc(sizeof(Graph));
    subg->vertices = (int*) malloc(vcLength * sizeof(int));
    subg->numOfVertices = 0;

    subg->end = (int*) malloc(g->endLength * sizeof(int));
    subg->endLength = 0;

    subg->start = (int*) malloc(g->startLength * sizeof(int));
    subg->startLength = 0;

    subg->startPointer = (int*) malloc(g->startPointerLength * sizeof(int));
    subg->startPointerLength = 0;

    for(int startIndex=0;startIndex<g->startLength;startIndex++){
        bool startInSubgraph = false;
        int startid = g->start[startIndex];

        //if startid is in vc
        if(!notInArray(vc, vcLength, startid)){
            int ifinish = startIndex + 1 < g->startPointerLength ? g->startPointer[startIndex+1] : g->endLength;

            for(int endIndex=g->startPointer[startIndex];endIndex<ifinish;endIndex++){
                int endid = g->end[endIndex];

                //if endid is in vc
                if(!notInArray(vc, vcLength, endid)){
                    if(!startInSubgraph){
                        subg->start[subg->startLength++] = startid;
                        startInSubgraph = true;

                        subg->startPointer[subg->startPointerLength++] = subg->endLength;

                        if(notInArray(subg->vertices, subg->numOfVertices, startid))
                        subg->vertices[subg->numOfVertices++] = startid;
                    }
                    subg->end[subg->endLength++] = endid;

                    if(notInArray(subg->vertices, subg->numOfVertices, endid))
                        subg->vertices[subg->numOfVertices++] = endid;
                }
            }
        }
    }

    resizeArray(subg->end, subg->endLength);
    resizeArray(subg->start, subg->startLength);
    resizeArray(subg->startPointer, subg->startPointerLength);

    // printf("Subgraph:\n");
    // printGraph(subg);

    return subg;
}

Array* findSccOfColor(Graph* g, int* vertexColor, int color){
    int* vc = (int*) malloc(g->numOfVertices * sizeof(int));
    int vcLength = 0;

    for(int i=0;i<g->numOfVertices;i++){
        if(vertexColor[i] == color){
            vc[vcLength++] = g->vertices[i];
        }
    }
    resizeArray(vc, vcLength);

    // printf("VC: ");
    // printArray(vc, vcLength);

    Graph* subg = createSubgraph(g, vc, vcLength);

    Array* scc = bfs(subg, color);
    // printf("SCC: ");
    // printArray(scc->arr, scc->length);

    free(vc);
    free(subg);

    return scc;
}

int colorScc(Graph* g){
    int sccCounter = 0;

    printf("Trimming...\n");
    //TODO: fix trimming
    sccCounter += trimGraph(g);
    printf("Trimming ended\n");

    int n = g->numOfVertices;
    int* vertexColor = (int*) malloc(n * sizeof(int));

    while(g->numOfVertices > 0){
        if(g->numOfVertices == 1){
            sccCounter++;
            break;
        }

        printf("Start\n");
        //printGraph(g);
        // printf("Vertex Color: ");
        // printArray(vertexColor, g->numOfVertices);
        printf("NumOfVertices=%d\n", g->numOfVertices);

        n = g->numOfVertices;
        for(int i=0;i<n;i++){
            vertexColor[i] = -1;
        }

        //Init each vertex color withe the vertex id
        //Can be done in Parallel
        for(int i=0;i<n;i++){
            initColor(vertexColor, i, g->vertices[i]);
        }

        // printf("Vertex Color: ");
        // printArray(vertexColor, g->numOfVertices);

        //Spread vertex color fw until there are no changes in vertexColor
        bool changedColor = true;
        while(changedColor){
            printf("Spreading color...\n");
            changedColor = false;
            
            //Can be done in Parallel
            for(int i=0;i<n;i++){
                //TODO
                //spreadColor();
                int color = vertexColor[i];
                int vid = g->vertices[i];

                //Get all the adjacent vertices of s and enqueue them if not visited
                int startIndex = getIndexOfValue(g->start, g->startLength, vid);

                //if vertex is a start of an edge
                if(startIndex != -1){
                    int ifinish = startIndex + 1 < g->startPointerLength ? g->startPointer[startIndex+1] : g->endLength;

                    for(int endIndex=g->startPointer[startIndex];endIndex<ifinish;endIndex++){
                        int endvid = g->end[endIndex];

                        int nextColorIndex = getIndexOfValue(g->vertices, g->numOfVertices, endvid);
                        //If vertex index has been removed
                        if(nextColorIndex == -1){
                            continue;
                        }

                        int nextColor = vertexColor[nextColorIndex];

                        if(nextColor < color){
                            vertexColor[i] = vertexColor[nextColorIndex];
                            changedColor = true;
                        }
                    }
                }
            }
        }
        printf("Spreading color ended\n");

        Array* uc = findUniqueColors(vertexColor, n);

        // printf("Unique colors: ");
        // printArray(uc->arr, uc->length);

        printf("Finding scc number...\n");
        for(int i=0;i<uc->length;i++){
            // printf("Vertex Color: ");
            // printArray(vertexColor, g->numOfVertices);

            int color = uc->arr[i];

            // printf("Color:%d\n", color);
            //Find all vertexes with color and put them in vc
            Array* scc = findSccOfColor(g, vertexColor, color);

            // printf("SccLength=%d", scc->length);
            if(scc->length != 0){
                sccCounter++;

                //Delete each vertex with if found in scc
                for(int j=0;j<scc->length;j++){
                    int vid = scc->arr[j];
                    deleteVertexFromGraph(g, vertexColor, vid);
                }
                // printf("\n");
                resizeArray(g->vertices, g->numOfVertices);
                resizeArray(vertexColor, g->numOfVertices);
            }
            free(scc);
        }
        printf("NumOfVertices=%d\n", g->numOfVertices);
        printf("Scc found\n");
        free(uc);
        //free(vertexColor);
    }
    return sccCounter;
}

int main(int argc, char** argv){
    CooArray* ca = (CooArray*) malloc(sizeof(CooArray));

    ca->i = (int[11]){1, 8, 2, 3, 4, 0, 5, 6, 4, 7, 9};
    ca->iLength = 11;
    ca->j = (int[11]){0, 0, 1, 2, 2, 3, 4, 5, 6, 6, 8};
    ca->jLength = 11;
    ca->numOfVertices = 10;

    ca = NULL;
    //ca = readMtxFile("graphs/celegansneural.mtx");
    ca = readMtxFile("graphs/foldoc.mtx");
    //ca = readMtxFile("graphs/language.mtx");
    //ca = readMtxFile("graphs/eu-2005.mtx");
    Graph* g = initGraphFromCoo(ca);
    //printGraph(g);

    time_t start, stop;
    start = time(NULL);

    int numOfScc = colorScc(g);

    stop = time(NULL);

    printf("Result=%d\n", numOfScc);

    printf("The number of seconds for colorScc to run was %ld\n", stop - start);

    free(g);

    return 0;
}