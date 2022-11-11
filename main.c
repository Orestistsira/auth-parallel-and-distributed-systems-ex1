#include "bfs.h"
#include "mmio.h"

typedef struct CooArray{
    int* i;
    int* j;

    int iLength;
    int jLength;
}CooArray;

void printArray(int* array, int n){
    for(int i=0;i<n;i++){
        printf("%d ", array[i]);
    }
    printf("\n");
}

void bfsTest(Graph* g){
    SccList* sccList = bfs(g, 2);

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

	return cooArray;
}

Graph* initGraphFromCoo(CooArray* ca){
    Graph* g = (Graph*) malloc(sizeof(Graph));
    g->end = ca->i;
    g->endLength = ca->iLength;

    g->start = (int*) malloc(ca->jLength * sizeof(int));
    g->startLength = 0;
    g->startPointer = (int*) malloc(ca->jLength * sizeof(int));
    g->startPointerLength = 0;

    int vid = -1;
    for(int index=0;index<ca->jLength;index++){
        if(vid != ca->j[index]){
            vid = ca->j[index];
            g->start[g->startLength] = vid;
            g->startLength++;
            g->startPointer[g->startPointerLength] = index;
            g->startPointerLength++;
        }
    }
    free(ca);

    return g;
}

int main(int argc, char** argv){
    Graph* g = (Graph*) malloc(sizeof(Graph));

    g->numOfVertices = 4;
    g->endLength = 6;
    g->startLength = 4;

    //Init Graph
    g->end = (int[6]){1, 2, 2, 0, 3, 3};
    g->start = (int[4]){0, 1, 2, 3};
    g->startPointer = (int[4]){0, 2, 3, 5};
    g->vertices = (int[4]){0, 1, 2, 3};

    printArray(g->end, g->endLength);
    printArray(g->start, g->startLength);

    //bfsTest(g);
    CooArray* ca = readMtxFile("graphs/celegansneural.mtx");
    printf("%d\n", ca->iLength);
    printf("%d\n", ca->jLength);

    //printArray(ca->i, ca->iLength);
    //printArray(ca->j, ca->jLength);

    Graph* g1 = initGraphFromCoo(ca);
    
    printArray(g1->end, g1->endLength);
    printf("---------------------------------------------------------------------------------------\n");
    printArray(g1->start, g1->startLength);
    printf("---------------------------------------------------------------------------------------\n");
    printArray(g1->startPointer, g1->startPointerLength);

    free(g);

    return 0;
}