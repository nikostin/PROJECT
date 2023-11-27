#ifndef GNNS_H
#define GNNS_H

#include "../lsh/lsh.h"
#include "../../lists/list.h"

typedef struct {
    LSH* lsh;                // LSH structure
    int k;                   // Number of nearest neighbors in k-NN graph
    int datasetSize;         // Size of the dataset
    int T;                   // Number of greedy steps
    int randomRestarts;      // Number of random restarts
    int E;                   // Number of expansions
    double R;                // Radius for GNNS
    List** kNearestNeighborsGraph; // k-NN graph
} GNNS;

GNNS* initGNNS(LSH* lsh, List* dataset, int k, int T, int randomRestarts, int E, double R);
List* queryGNNS(GNNS* gnns, Vector q, int N);
void freeGNNS(GNNS* gnns);

#endif // GNNS_H

