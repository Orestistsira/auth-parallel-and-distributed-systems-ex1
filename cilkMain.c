#include <sys/time.h>

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

    struct timeval startwtime, endwtime;

    gettimeofday (&startwtime, NULL);

    int numOfScc = 0;
    numOfScc = cilkColorScc(g, true);

    gettimeofday (&endwtime, NULL);

    double duration = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);

    printf("[Result=%d]\n", numOfScc);

    printf("[ColorScc took %.4f seconds]\n", duration);

    free(g);

    return 0;
}