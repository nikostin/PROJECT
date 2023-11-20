#include "macqueen.h"
#include <stdlib.h>
#include <math.h>
#include "../../search/distancemetrics/euclidean.h"  
#include "../../lists/list.h"
#include "../../vectors/vector.h"
#include <stdio.h>


static double getDistance(Vector a, Vector b) {
    return euclideanDistance(a, b, 784);  // Calculate the distance using the Euclidean distance function for MNIST
}

List* macQueenUpdate(List* data, List* centroids) {
    //printf("[DEBUG]Entering macQueenUpdate\n");

    if (!data || !centroids) {
        printf("macQueenUpdate: data or centroids is NULL\n");
        return NULL;
    }

    int K = lengthOfList(centroids); 
    printf("macQueenUpdate: Number of centroids (K): %d\n", K);

    ClusterNode* clusters = (ClusterNode*)malloc(K * sizeof(ClusterNode));

    Node* node = centroids->head;
    for (int i = 0; i < K; i++) {
        clusters[i].centroid = node->vector;
        clusters[i].count = 0;
        node = node->next;
        //printf("[DEBUG]Initialized cluster %d with centroid\n", i);
    }

    node = data->head;
    int unassignedDataPoints = 0;
    while (node) {
        int minIndex = -1;
        double minDist = INFINITY;

        for (int i = 0; i < K; i++) {
            double dist = getDistance(node->vector, clusters[i].centroid);
            if (dist < minDist) {
                minDist = dist;
                minIndex = i;
            }
        }

        if (minIndex >= 0) {
            clusters[minIndex].count += 1;

            // Simple average approach
            for (int j = 0; j < 784; j++) {
                clusters[minIndex].centroid.coordinates[j] += node->vector.coordinates[j];
            }
            
           // //printf("[DEBUG]Assigned data point %s to Centroid %d\n", node->vector.name, minIndex);
        } else {
            printf("Data point %s was not assigned to any cluster!\n", node->vector.name);
            unassignedDataPoints++;
        }
        
        node = node->next;
    }

    // Check for clusters with zero data points and handle
    for (int i = 0; i < K; i++) {
        if (clusters[i].count == 0) {
            //printf("[DEBUG]Cluster %d has no data points assigned. Handling...\n", i);
            // Handle this by choosing a random point or another strategy
       
            int randomIndex = rand() % lengthOfList(data);
            Node* randomDataPoint = getNthNode(data, randomIndex + 1);  // +1 because the function seems to be 1-indexed
            clusters[i].centroid = randomDataPoint->vector;
        } else {
            // Finish the simple averaging
            for (int j = 0; j < 784; j++) {
                clusters[i].centroid.coordinates[j] /= clusters[i].count;
            }
        }

        // Print centroid coordinates and cluster size
       // printf("Centroid %d coordinates:\n", i);
       // for (int j = 0; j < 784; j++) {
            //printf("[DEBUG mcqueen coordinates %f ", clusters[i].centroid.coordinates[j]);
       // }
        //printf("\n Initial Cluster %d size: %d\n", i, clusters[i].count);
    }

    if (unassignedDataPoints > 0) {
        printf("macQueenUpdate: %d data points were not assigned to any cluster!\n", unassignedDataPoints);
    }

        List* updatedCentroids = initializeList();
    for (int i = 0; i < K; i++) {
        sprintf(clusters[i].centroid.name, "Centroid%d", i);  // Assign new names
        insertToList(updatedCentroids, clusters[i].centroid);
    }

    free(clusters);

    //printf("[DEBUG]Exiting macQueenUpdate\n");
    return updatedCentroids;
}
