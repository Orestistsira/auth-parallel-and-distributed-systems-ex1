#include <string.h>

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

void resizeArray(int* arr, int newSize){
    int* temp = (int*) realloc(arr, newSize * sizeof(int));
    arr = temp;
}

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
    SccList* sccList = bfs(g, source);

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

    if (f !=stdin) fclose(f);

    /************************/
    /* now write out matrix */
    /************************/

    mm_write_banner(stdout, matcode);
    mm_write_mtx_crd_size(stdout, M, N, nz);
    for(i=0; i<nz; i++){
        fprintf(stdout, "%d %d %20.19g\n", I[i]+1, J[i]+1, val[i]);
    }

    CooArray* cooArray = (CooArray*) malloc(sizeof(CooArray));
    cooArray->i = I;
    cooArray->j = J;
    cooArray->iLength = nz;
    cooArray->jLength = nz;
    cooArray->numOfVertices = M;

	return cooArray;
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

int main(int argc, char** argv){
    CooArray* ca = (CooArray*) malloc(sizeof(CooArray));

    ca->i = (int[9]){1, 2, 3, 4, 0, 5, 6, 4, 7};
    ca->iLength = 9;
    ca->j = (int[9]){0, 1, 2, 2, 3, 4, 5, 6, 6};
    ca->jLength = 9;
    ca->numOfVertices = 8;

    //ca = NULL;
    //ca = readMtxFile("graphs/celegansneural.mtx");
    Graph* g = initGraphFromCoo(ca);
    
    printArray(g->end, g->endLength);
    printf("---------------------------------------------------------------------------------------\n");
    printArray(g->start, g->startLength);
    printf("---------------------------------------------------------------------------------------\n");
    printArray(g->startPointer, g->startPointerLength);
    printf("---------------------------------------------------------------------------------------\n");
    printArray(g->vertices, g->numOfVertices);
    printf("NoV=%d\n", g->numOfVertices);

    bfsTest(g, 0);

    //free(g);
    free(g);

    return 0;
}