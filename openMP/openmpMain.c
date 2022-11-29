#include <sys/time.h>

#include "openmpScc.h"

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
    printf("Initializing Graph...\n");
    Graph* g = initGraphFromCoo(ca);
    printf("Graph ready.\n");
    //printGraph(g);

    int numOfThreads = omp_get_num_threads();
    printf("Number of threads = %d\n", numOfThreads);

    struct timeval startwtime, endwtime;

    gettimeofday (&startwtime, NULL);

    int numOfScc = 0;
    numOfScc = openmpColorScc(g, true);

    gettimeofday (&endwtime, NULL);

    double duration = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);

    printf("[Result=%d]\n", numOfScc);

    printf("[ColorScc took %.4f seconds]\n", duration);

    free(g->start);
    free(g->startPointer);
    free(g->vertices);
    free(g->vertexPosInStart);
    free(g->end);
    free(g->inDegree);
    free(g->outDegree);
    free(g);

    return 0;
}