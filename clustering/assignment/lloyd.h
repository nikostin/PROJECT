#ifndef LLOYD_H
#define LLOYD_H

#include "../../lists/list.h"


typedef struct ClusteredNode {
    Node baseNode;              // This will hold the vector and next pointer
    struct ClusteredNode* assignedCentroid;
} ClusteredNode;

Node* findClosestCentroidForVector(Vector* dataVector, List* centroids);

// Function to assign data points to clusters using Lloyd's algorithm
List* assignWithLloyd(List* data, List* centroids);



#endif // LLOYD_H
