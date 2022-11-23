#include <string.h>
#include <sys/time.h>

//#include "seqScc.h"
#include "parallelScc.h"

int main(int argc, char** argv){
    // CooArray* ca = (CooArray*) malloc(sizeof(CooArray));

    // ca->i = (int[9]){1, 2, 3, 4, 0, 5, 6, 4, 7};
    // ca->iLength = 9;
    // ca->j = (int[9]){0, 1, 2, 2, 3, 4, 5, 6, 6};
    // ca->jLength = 9;
    // ca->numOfVertices = 8;

    // ca = NULL;

    char* filename = NULL;
    bool parallel = false;
    bool trimming = false;
    int givenNumOfThreads = 10;
    if(argc == 5){
        filename = argv[1];
        
        if(!strcmp("trimming", argv[2]))
            trimming = true;
        else if(!strcmp("no-trimming", argv[2]))
            trimming = false;
        else
            printf("Error in arguments!\n");


        if(!strcmp("parallel", argv[3]))
            parallel = true;
        else if(!strcmp("sequential", argv[3]))
            parallel = false;
        else
            printf("Error in arguments!\n");


        givenNumOfThreads = atoi(argv[4]);

    }
    else if(argc == 2){
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

    struct timeval startwtime, endwtime;

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

    double duration = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);

    printf("[Result=%d]\n", numOfScc);

    printf("[ColorScc took %.4f seconds]\n", duration);

    free(g);

    return 0;
}