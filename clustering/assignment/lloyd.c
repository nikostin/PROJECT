#include "lloyd.h"
#include "../../search/distancemetrics/euclidean.h"
#include "../../lists/list.h"
#include <stddef.h>  // for NULL
#include <math.h>    // for INFINITY
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>



// Helper function to find the closest centroid to a given data point
Node * findClosestCentroidForVector(Vector* dataVector, List* centroids) {
    Node * centroidNode = centroids->head;
    Node * closestCentroid = NULL;
    double minDistance = INFINITY;

    while (centroidNode) {
        double dist = euclideanDistance(*dataVector, centroidNode->vector, 784);
        if (dist < minDistance) {
            minDistance = dist;
            closestCentroid = centroidNode;
        }
        centroidNode = centroidNode->next;
    }

    return closestCentroid;
}

List* assignWithLloyd(List* data, List* centroids) {
    //printf("[DEBUG] Entering assignWithLloyd\n");

    ClusteredNode* dataNode = (ClusteredNode*)data->head;
    int dataIndex = 0;
    
    while (dataNode) {
       // printf("[DEBUG] Processing data node %d\n", dataIndex);
        
        ClusteredNode* closestCentroid = (ClusteredNode*)findClosestCentroidForVector(&dataNode->baseNode.vector, centroids);
       // printf("[DEBUG] Closest centroid found for data node %d\n", dataIndex);

        if (closestCentroid) {
            dataNode->assignedCentroid = closestCentroid;
            //printf("[DEBUG] Data node %d assigned to centroid: %s\n", dataIndex, closestCentroid->baseNode.vector.name); 
        } else {
            printf("Error: No closest centroid found for data node %d\n", dataIndex);
        }

        dataIndex++;
        dataNode = (ClusteredNode*)dataNode->baseNode.next;
    }

    //printf("[DEBUG] Exiting assignWithLloyd\n");
    return centroids;  // Return the centroids list, with data nodes now pointing to their respective centroids.
}
