#include <string.h>
#include <time.h>

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
    if(argc == 3){
        filename = argv[1];
        if(!strcmp("parallel", argv[2]))
            parallel = true;
        else if(!strcmp("sequential", argv[2]))
            parallel = false;
        else
            printf("Error in arguments!\n");
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
    Graph* g = initGraphFromCoo(ca);
    //printGraph(g);

    time_t begin = time(NULL);

    int numOfScc = 0;
    if(parallel){
        printf("Starting parallel algorithm...\n");
        numOfScc = parallelColorScc(g);
    }
    else{
        printf("Starting sequential algorithm...\n");
        numOfScc = sequentialColorScc(g);  
    }

    time_t end = time(NULL);

    printf("[Result=%d]\n", numOfScc);

    printf("[ColorScc took %ld seconds]\n", (end - begin));

    free(g);

    return 0;
}