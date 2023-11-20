#include "kmeanspp.h"
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "../../search/distancemetrics/euclidean.h" 
#include "../../lists/list.h"
#include "../../vectors/vector.h"
#include <stdio.h>



static double squaredEuclideanDistance(Vector a, Vector b) {
    // Returns the squared Euclidean distance between two vectors
    return pow(euclideanDistance(a, b, 784), 2); 
}

List* kmeansPlusPlusInitialization(List* data, int K) {
    ////printf("[DEBUG]Entering kmeansPlusPlusInitialization with K = %d\n", K);
    if (!data || K <= 0) {
        printf("kmeansPlusPlusInitialization: data is NULL or K is invalid\n");
        return NULL;
    }

    List* centroids = initializeList();
    ////printf("[DEBUG]Centroids list initialized\n");

    int randomIndex = rand() % lengthOfList(data);
    Vector* firstCentroid = getVectorAt(data, randomIndex);
    printf("kmeansPlusPlusInitialization: First centroid chosen: %s\n", firstCentroid->name);

    if (!firstCentroid) {
        freeList(centroids);
        return NULL;
    }
    
    insertToList(centroids, *firstCentroid);
    ////printf("[DEBUG]First centroid inserted to the list\n");

    double* distances = (double*)malloc(lengthOfList(data) * sizeof(double));
    ////printf("[DEBUG]Distances array initialized with size %d\n", lengthOfList(data));

    for (int i = 1; i < K; i++) {
        double sum = 0;
        Node* node = data->head;
        int dataIndex = 0;
        while (node) {
            double minDist = DBL_MAX;
            Node* centroidNode = centroids->head;
            while (centroidNode) {
                double dist = squaredEuclideanDistance(node->vector, centroidNode->vector);
                if (dist < minDist) {
                    minDist = dist;
                }
                centroidNode = centroidNode->next;
            }
            distances[dataIndex] = minDist;
            sum += minDist;
            ////printf("[DEBUG]Data node %d: minDist = %f, current sum = %f\n", dataIndex, minDist, sum);
            node = node->next;
            dataIndex++;
        }

        double rnd = ((double)rand() / RAND_MAX) * sum;
        ////printf("[DEBUG]Random number generated for next centroid = %f\n", rnd);
        for (dataIndex = 0; dataIndex < lengthOfList(data); dataIndex++) {
            if ((rnd -= distances[dataIndex]) <= 0) {
                break;
            }
        }

        Vector* nextCentroid = getVectorAt(data, dataIndex);
        printf("kmeansPlusPlusInitialization: Next centroid chosen: %s\n", nextCentroid->name);
        insertToList(centroids, *nextCentroid);
    }

    free(distances);
    ////printf("[DEBUG]Exiting kmeansPlusPlusInitialization\n");
    return centroids;
}
