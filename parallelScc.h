#include <pthread.h>

#include "seqScc.h"

int parallelColorScc(Graph* g, bool trimming, int givenNumOfThreads);