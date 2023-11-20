#include "silhouette.h"
#include "../../search/distancemetrics/euclidean.h"
#include "../../lists/list.h"
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h> // Include OpenMP for parallelization

#define BAR_LENGTH 50  // Length of the progress bar in characters

// Display a progress bar

void displayProgressBar(int progress, int total) {
    int position = (int)((double) BAR_LENGTH * progress / total);
    printf("[");
    for (int i = 0; i < BAR_LENGTH; i++) {
        if (i < position) {
            printf("#");
        } else {
            printf(" ");
        }
    }
    printf("] %d%%\r", (int)(100.0 * progress / total));
    fflush(stdout);
}

// Function to compute the average distance from the i-th data point 
// to the other data points in the same cluster
double compute_a(Node* dataNode, List* cluster) {
    double sum = 0.0;
    int count = 0;
    Node* currentNode = cluster->head;
    while (currentNode) {
        // Exclude the dataNode from its own average calculation
        if (currentNode != dataNode) {
            sum += euclideanDistance(dataNode->vector, currentNode->vector, 784);
            count++;
        }
        currentNode = currentNode->next;
    }

    double avg = (count > 0) ? sum / count : 0.0;
   // //printf("[DEBUG]compute_a: Average distance for data node = %f\n", avg);
    return avg;
}

// Function to compute the smallest average distance from the i-th data point 
// to the data points in a different cluster, minimized over clusters
double compute_b(Node* dataNode, List* data, List* centroids, List** clusters) {
    double minAvgDistance = INFINITY;
    Node* centroidNode = centroids->head;
    for (int i = 0; centroidNode != NULL; i++) {
        if (!isDataNodeInCluster(dataNode, clusters[i])) {
            double avgDistance = compute_a(dataNode, clusters[i]);
            if (avgDistance < minAvgDistance) {
                minAvgDistance = avgDistance;
            }
        }
        centroidNode = centroidNode->next;
    }
    return minAvgDistance;
}
// A simple min function to get the smaller of two integers
int min(int a, int b) {
    return (a < b) ? a : b;
}


#include <stdlib.h> // For malloc

double* silhouetteScore(List* data, List* centroids) {
    int numCentroids = lengthOfList(centroids);
    List* clusters[numCentroids];
    double* silhouetteScores = (double*)malloc((numCentroids + 1) * sizeof(double)); // Space for individual and total scores

    // Initialize clusters and silhouetteScores
    for (int i = 0; i < numCentroids; i++) {
        clusters[i] = initializeList();
        silhouetteScores[i] = 0.0; // Initialize individual cluster score
    }
    silhouetteScores[numCentroids] = 0.0; // Initialize total score

    // Assign data nodes to their respective clusters
    Node* currentNode = data->head;
    while (currentNode) {
        int clusterIndex = findClusterIndexForDataNode(currentNode, centroids);
        insertToList(clusters[clusterIndex], currentNode->vector);
        currentNode = currentNode->next;
    }

    int totalDataNodes = lengthOfList(data);
    int testLimit = 100; // Only measure the first 100 data nodes
    int upperLimit = min(testLimit, totalDataNodes);

    #pragma omp parallel for reduction(+:silhouetteScores[:numCentroids+1])
    for (int i = 0; i < upperLimit; i++) {
        Node* dataNode = getNthNode(data, i + 1);
        int clusterIndex = findClusterIndexForDataNode(dataNode, centroids);
        List* dataNodeCluster = clusters[clusterIndex];
        double a = compute_a(dataNode, dataNodeCluster);
        double b = compute_b(dataNode, data, centroids, clusters);

        double s = (b - a) / (a > b ? a : b);

        #pragma omp critical
        {
            silhouetteScores[clusterIndex] += s;
            silhouetteScores[numCentroids] += s; // Add to total score
        }
    }

    // Average scores for each cluster
    for (int i = 0; i < numCentroids; i++) {
        int clusterSize = lengthOfList(clusters[i]);
        silhouetteScores[i] /= clusterSize;
    }

    // Average total score
    silhouetteScores[numCentroids] /= totalDataNodes;

    // Free clusters
    for (int i = 0; i < numCentroids; i++) {
        freeList(clusters[i]);
    }

    return silhouetteScores;
}

int isDataNodeInCluster(Node* dataNode, List* cluster) {
        //printf("[DEBUG]Entering isDataNodeInCluster. Checking if data node is part of the given cluster.\n");


    Node* currentNode = cluster->head;
    while (currentNode) {
        if (dataNode == currentNode) {  // Comparing pointers,  unique nodes for each data point
            //printf("[DEBUG]Data node found in the cluster\n");
            return 1;  // True
        }
        currentNode = currentNode->next;
    }

    //printf("[DEBUG]Exiting isDataNodeInCluster. Result: %d\n", (currentNode == NULL ? 0 : 1));
    return currentNode == NULL ? 0 : 1;
}




int findClusterIndexForDataNode(Node* dataNode, List* centroids) {
    ////printf("[DEBUG]Entering findClusterIndexForDataNode\n");

    Node* closestCentroid = findClosestCentroid(dataNode, centroids);
    Node* centroidNode = centroids->head;
    int index = 0;
    while (centroidNode) {
        if (centroidNode == closestCentroid) {
            return index;
        }
        centroidNode = centroidNode->next;
        index++;
    }

    ////printf("[DEBUG]Undfortunately Exiting findClusterIndexForDataNode\n");
    return -1;  // Should not reach this, added for safety
}

// Function to find the centroid closest to a given dataNode
Node* findClosestCentroid(Node* dataNode, List* centroids) {
    Node* centroidNode = centroids->head;
    Node* closestCentroid = NULL;
    double minDistance = INFINITY;
    while (centroidNode) {
        double currentDistance = euclideanDistance(dataNode->vector, centroidNode->vector, 784);
        if (currentDistance < minDistance) {
            minDistance = currentDistance;
            closestCentroid = centroidNode;
        }
        centroidNode = centroidNode->next;
    }




    return closestCentroid;
}

List* retrieveDataPointsForCentroid(Node* centroid, List* data, List* centroids) {
    ////printf("[DEBUG]Entering retrieveDataPointsForCentroid\n");

    List* cluster = initializeList();
    Node* dataNode = data->head;
    int nodeCounter = 0;  // Counter for the number of nodes processed
    //int totalNodes = lengthOfList(data);  // count the nodes in the list

    while (dataNode) {
        // Update the debug message with node details and progress
        ////printf("[DEBUG]Processing data node %d/%d: ", nodeCounter + 1, totalNodes);
        // If dataNode has an identifier, print it here. For example: dataNode->id or dataNode->name

        if (findClosestCentroid(dataNode, centroids) == centroid) {
            ////printf("[DEBUG]Adding data node to cluster\n");
            insertToList(cluster, dataNode->vector);  // Add dataNode to the cluster list
        } else {
            ////printf("[DEBUG]Data node not added to cluster\n");
        }

        dataNode = dataNode->next;
        nodeCounter++;

        if (!dataNode) {
           // //printf("[DEBUG]Reached end of data list\n");
            break;
        }
    }

    ////printf("[DEBUG]Exiting retrieveDataPointsForCentroid after processing %d nodes\n", nodeCounter);
    return cluster;
}



