#ifndef MRNG_H
#define MRNG_H
#include "../distancemetrics/euclidean.h"
#include "../lsh/lsh.h"
#include "../../lists/list.h"
#include "../../vectors/vector.h"

// Struct representing a node in the MRNG
typedef struct MRNGNode {
    Vector vector; // The vector stored at this node
    List* neighbors; // List of neighbors for this vector in the MRNG
} MRNGNode;

// Struct representing the MRNG itself
typedef struct MRNG {
    int size; // Number of nodes in the MRNG
    MRNGNode* nodes; // Array of MRNG nodes
} MRNG;

// Function prototypes

// Initialize MRNG with a dataset and LSH parameters
MRNG* initMRNG(List* dataset, LSH* lsh, int l, int N, double R_lsh);

// Query MRNG for N nearest neighbors to a given vector
List* queryMRNG(MRNG* mrng, Vector queryVector, int N);

// Free resources allocated for MRNG
void freeMRNG(MRNG* mrng);

// Utility function to find the initial node for MRNG query
MRNGNode* findInitialNode(MRNG* mrng, Vector queryVector);

List* copyList(List* original);
Vector* createDatasetArray(List* dataset, Vector currentVector);

#endif // MRNG_H
