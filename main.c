#include <string.h>
#include <time.h>

#include "sequentialScc.h"

int main(int argc, char** argv){
    // CooArray* ca = (CooArray*) malloc(sizeof(CooArray));

    // ca->i = (int[9]){1, 2, 3, 4, 0, 5, 6, 4, 7};
    // ca->iLength = 9;
    // ca->j = (int[9]){0, 1, 2, 2, 3, 4, 5, 6, 6};
    // ca->jLength = 9;
    // ca->numOfVertices = 8;

    // ca = NULL;

    char* filename = NULL;
    if(argc == 2){
        filename = argv[1];
    }
    else{
        printf("Error in arguments!\n");
        exit(1);
    }

    //ca = readMtxFile("graphs/celegansneural.mtx");
    CooArray* ca = readMtxFile(filename);
    //ca = readMtxFile("graphs/language.mtx");
    //ca = readMtxFile("graphs/eu-2005.mtx");
    Graph* g = initGraphFromCoo(ca);
    //printGraph(g);

    clock_t t;
    t = clock();

    int numOfScc = sequentialColorScc(g);

    t = clock() - t;

    printf("[Result=%d]\n", numOfScc);

    printf("[ColorScc took %.4f seconds]\n", ((double)t)/CLOCKS_PER_SEC);

    free(g);

    return 0;
}