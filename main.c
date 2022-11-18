#include <string.h>
#include <time.h>

#include "sequentialScc.h"

// void resizeArray(int* arr, int newSize){
//     int* temp = (int*) realloc(arr, newSize * sizeof(int));
//     arr = temp;

// }

int main(int argc, char** argv){
    // CooArray* ca = (CooArray*) malloc(sizeof(CooArray));

    // ca->i = (int[11]){1, 8, 2, 3, 4, 0, 5, 6, 4, 7, 9};
    // ca->iLength = 11;
    // ca->j = (int[11]){0, 0, 1, 2, 2, 3, 4, 5, 6, 6, 8};
    // ca->jLength = 11;
    // ca->numOfVertices = 10;

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