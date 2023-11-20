#include "hypercube_reverse_search.h"
#include "../../search/hypercubes/hypercube.h"
#include "../../lists/list.h"
#include "../cluster.h"  
#include <stdio.h>
#include <math.h>

List* assignWithHypercubeReverseSearch(List* data, List* centroids, Hypercube* hypercube, int N, double R) {
    //printf("[DEBUG] Entering assignWithHypercubeReverseSearch\n");

    // Create a list to store the clusters
    List* clusters = initializeList();

    // Iterate over centroids and initialize clusters
    Node* centroidNode = centroids->head;
    while (centroidNode) {
        Cluster* newCluster = initCluster(centroidNode->vector);
        insertClusterToList(clusters, newCluster); // Assume this function exists to add Cluster to a List
        centroidNode = centroidNode->next;
    }

    // Iterate over data and assign each item to a cluster using Hypercube
    Node* dataNode = data->head;
    while (dataNode) {
        // Use Hypercube to find the nearest centroid
        List* nearestCentroids = queryHypercube(hypercube, dataNode->vector, 1, hypercube->probes, N, R);

        if (nearestCentroids && nearestCentroids->head) {
            Vector nearestCentroid = nearestCentroids->head->vector;

            // Find the corresponding cluster
            Cluster* foundCluster = findClusterByCentroid(clusters, nearestCentroid);
            if (foundCluster) {
                // Add the data vector to the found cluster
                addVectorToCluster(foundCluster, dataNode->vector);
            }
        }

        dataNode = dataNode->next;
    }

    //printf("[DEBUG] Exiting assignWithHypercubeReverseSearch\n");
    return clusters;
}
