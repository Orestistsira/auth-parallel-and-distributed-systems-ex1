#include <time.h>

#include "cilkScc.h"

int main(int argc, char** argv){
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

    time_t begin = time(NULL);

    int numOfScc = 0;
    numOfScc = cilkColorScc(g, true);

    time_t end = time(NULL);

    printf("[Result=%d]\n", numOfScc);

    printf("[ColorScc took %ld seconds]\n", (end - begin));

    free(g);

    return 0;
}