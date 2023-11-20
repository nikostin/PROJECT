#ifndef SILHOUETTE_H
#define SILHOUETTE_H

#include "../../lists/list.h"
#include "../assignment/lloyd.h"

// Cache structure for memoization
typedef struct {
    int* valid;       // Array to track which indices are valid (already computed)
    double* a_values; // Cached a values
    double* b_values; // Cached b values
} Cache;

// Function to initialize the cache
Cache* initializeCache(int size);

// Function to free the cache memory
void freeCache(Cache* cache);

// Function to find the centroid closest to a given dataNode
Node* findClosestCentroid(Node* dataNode, List* centroids);

// Function to calculate the silhouette score of the clustering
double* silhouetteScore(List* data, List* centroids);

// Retrieves all data points assigned to a given centroid
List* retrieveDataPointsForCentroid(Node* centroid, List* data, List* centroids);

// Checks if a given dataNode is present in a specific cluster
int isDataNodeInCluster(Node* dataNode, List* cluster);

// Function to find the index of the cluster for a given dataNode
int findClusterIndexForDataNode(Node* dataNode, List* centroids);

#endif // SILHOUETTE_H
