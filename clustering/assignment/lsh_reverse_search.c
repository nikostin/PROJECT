#include "lsh_reverse_search.h"
#include "../../search/lsh/lsh.h"
#include "../../lists/list.h"
#include "../cluster.h"  
#include <stdio.h>
#include <math.h>


#include "lsh_reverse_search.h"
#include "../../search/lsh/lsh.h"
#include "../../lists/list.h"
#include "../cluster.h"  
#include <stdio.h>
#include <math.h>

// Function to assign data vectors to clusters using LSH reverse search
List* assignWithLSHReverseSearch(List* data, List* centroids, LSH* lsh, int N, double R) {
    //printf("[DEBUG] Entering assignWithLSHReverseSearch\n");

    // Create a list to store the clusters
    List* clusters = initializeList();

    // Iterate over centroids and initialize clusters
    Node* centroidNode = centroids->head;
    while (centroidNode) {
        Cluster* newCluster = initCluster(centroidNode->vector);
        insertClusterToList(clusters, newCluster); // Assume this function exists to add Cluster to a List
        centroidNode = centroidNode->next;
    }

    // Iterate over data and assign each item to a cluster using LSH
    Node* dataNode = data->head;
    while (dataNode) {
        // Use LSH to find the nearest centroid with the provided N and R
        List* nearestCentroids = queryLSH(lsh, dataNode->vector, N, R);

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

    //printf("[DEBUG] Exiting assignWithLSHReverseSearch\n");
    return clusters;
}
