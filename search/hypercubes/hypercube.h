// /search/hypercubes/hypercube.h

#ifndef HYPERCUBE_H
#define HYPERCUBE_H

#include "../../lists/list.h"
#include "../../vectors/vector.h"
#include "../hashing/hashing.h"  

typedef struct {
    int k;                  // Number of hash functions
    int M;                  // Maximum permissible number of candidate points
    int probes;             // Maximum number of vertices to be checked
    int N;                  // Maximum number of nearest neighbors
    double R;               // Radius
    HashFunction* hashFunctions; // Array of hash functions
    List** buckets;              // Buckets of the hypercube
	double window;

} Hypercube;

// Initialize the hypercube
Hypercube* initHypercube(int k, int M, int probes, int N, double R, double window);

// Insert a vector into the hypercube
void insertIntoHypercube(Hypercube* hc, Vector vec);

// Query the hypercube to find similar vectors to the query vector 'q'
List* queryHypercube(Hypercube* hc, Vector q, int M, int probes, int N, double R);

// Free the memory associated with the hypercube
void freeHypercube(Hypercube* hc);

#endif  // HYPERCUBE_H
