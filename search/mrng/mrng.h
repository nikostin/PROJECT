#ifndef MRNG_H
#define MRNG_H

#include "../../lists/list.h"
#include "../../vectors/vector.h"
#include "../lsh/lsh.h"

// Structure to represent a node in the MRNG
typedef struct {
    Vector vector;
    List* neighbors; // List of neighbors for this node
} MRNGNode;

// Structure to represent the MRNG
typedef struct {
    MRNGNode* nodes; // Array of nodes
    int size;        // Number of nodes in the MRNG
    Vector navigatingNode; // Navigating node for the MRNG
} MRNG;

// Function declarations
MRNG* initMRNG(List* dataset, LSH* lsh, int l, int N, double R);
List* queryMRNG(MRNG* mrng, Vector query, int N);
void freeMRNG(MRNG* mrng);

#endif // MRNG_H
