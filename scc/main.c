#include <string.h>

#include "parallelScc.h"

int main(int argc, char** argv){
    char* filename = NULL;
    bool parallel = false;
    bool trimming = false;
    int givenNumOfThreads = 4;

    if(argc == 4 || argc == 5){
        filename = argv[1];
        
        if(!strcmp("trimming", argv[2]))
            trimming = true;
        else if(!strcmp("no-trimming", argv[2]))
            trimming = false;
        else{
            printf("Error in arguments!\n");
            exit(1);
        }

        if(!strcmp("parallel", argv[3])){
            parallel = true;
            if(argc == 5)
                givenNumOfThreads = atoi(argv[4]);
        }
        else if(!strcmp("sequential", argv[3]))
            parallel = false;
        else{
            printf("Error in arguments!\n");
            exit(1);
        }
    }
    else{
        printf("Error in arguments!\n");
        exit(1);
    }

    if(parallel)
        printf("Number of threads = %d\n", givenNumOfThreads);

    struct timeval startwtime, endwtime;
    double duration;

    //Read the mtx file
    printf("Loading graph...\n");
    CooArray* ca = readMtxFile(filename);

    //Initialize the graph from the COO array
    printf("Initializing Graph...\n");
    gettimeofday (&startwtime, NULL);
    Graph* g = initGraphFromCoo(ca);
    gettimeofday (&endwtime, NULL);
    duration = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);
    printf("Graph ready in %.4f seconds.\n", duration);

    
    gettimeofday (&startwtime, NULL);
    int numOfScc = 0;
    if(parallel){
        printf("Starting parallel algorithm...\n");
        numOfScc = parallelColorScc(g, trimming, givenNumOfThreads);
    }
    else{
        printf("Starting sequential algorithm...\n");
        numOfScc = sequentialColorScc(g, trimming);
    }

    gettimeofday (&endwtime, NULL);

    duration = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);

    printf("[Result=%d]\n", numOfScc);

    printf("[ColorScc took %.4f seconds]\n", duration);

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