#include <sys/time.h>

#include "openmpScc.h"

int main(int argc, char** argv){
    char* filename = NULL;
    bool trimming = false;
    if(argc == 3){
        filename = argv[1];
        
        if(!strcmp("trimming", argv[2]))
            trimming = true;
        else if(!strcmp("no-trimming", argv[2]))
            trimming = false;
        else{
            printf("Error in arguments!\n");
            exit(1);
        }
    }
    else{
        printf("Error in arguments!\n");
        exit(1);
    }

    printf("Loading graph...\n");
    CooArray* ca = readMtxFile(filename);
    printf("Initializing Graph...\n");
    Graph* g = initGraphFromCoo(ca);
    printf("Graph ready.\n");

    struct timeval startwtime, endwtime;

    gettimeofday (&startwtime, NULL);

    printf("Starting parallel openMP algorithm...\n");
    int numOfScc = 0;
    numOfScc = openmpColorScc(g, trimming);

    gettimeofday (&endwtime, NULL);

    double duration = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);

    printf("[Result=%d]\n", numOfScc);

    printf("[ColorScc took %.4f seconds]\n", duration);

    free(g->sccIdOfVertex);
    free(g->startAll);
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