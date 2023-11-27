#include "gnns.h"
#include "../lsh/lsh.h"
#include "../distancemetrics/euclidean.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>

// Helper function declarations
List* getNeighbors(List** kNearestNeighborsGraph, int nodeIndex, int E);
Vector findClosestNeighbor(List* neighbors, Vector q);
int isInList(List* list, Vector vec);
int isLocalOptimum(Vector current, List* neighbors, Vector q);
void truncateListToN(List* list, int N);

// Initializes GNNS and constructs the k-NN graph
GNNS* initGNNS(LSH* lsh, List* dataset, int k, int T, int randomRestarts, int E, double R) {
    GNNS* gnns = (GNNS*)malloc(sizeof(GNNS));
    if (!gnns) {
        perror("Failed to allocate memory for GNNS");
        exit(EXIT_FAILURE);
    }

    gnns->lsh = lsh;
    gnns->k = k;
    gnns->datasetSize = dataset->size;
    gnns->T = T; 
    gnns->randomRestarts = randomRestarts;  
    gnns->E = E;  
    gnns->R = R;

    gnns->kNearestNeighborsGraph = (List**)malloc(gnns->datasetSize * sizeof(List*));
    if (!gnns->kNearestNeighborsGraph) {
        perror("Failed to allocate memory for k-NN graph");
        free(gnns);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < gnns->datasetSize; i++) {
        Vector currentVector = *getVectorAt(dataset, i);
        gnns->kNearestNeighborsGraph[i] = initializeList();

        List* neighbors = queryLSH(gnns->lsh, currentVector, k, gnns->R);

        Node* currentNode = neighbors->head;
        while (currentNode && gnns->kNearestNeighborsGraph[i]->size < k) {
            insertToList(gnns->kNearestNeighborsGraph[i], currentNode->vector);
            currentNode = currentNode->next;
        }
        freeList(neighbors);
    }
    return gnns;
}

// Query function for GNNS
List* queryGNNS(GNNS* gnns, Vector q, int N) {
    if (!gnns || !gnns->lsh) {
        fprintf(stderr, "GNNS structure not initialized\n");
        return NULL;
    }

    List* results = initializeList();
    srand(time(NULL));  // Seed for random number generation

    printf("Running GNNS for query image: %s\n", q.name);
    for (int restart = 0; restart < gnns->randomRestarts; restart++) {
        int randomStartIndex = rand() % gnns->datasetSize;
        Vector current = *getVectorAt(gnns->kNearestNeighborsGraph[randomStartIndex], 0);

        for (int step = 0; step < gnns->T; step++) {
            List* neighbors = getNeighbors(gnns->kNearestNeighborsGraph, randomStartIndex, gnns->E);
            Vector closestNeighbor = findClosestNeighbor(neighbors, q);

            if (!isInList(results, closestNeighbor) && strcmp(closestNeighbor.name, q.name) != 0) {
                insertToList(results, closestNeighbor);
            }

            if (isLocalOptimum(current, neighbors, q) || results->size >= N) {
                break;
            }

            current = closestNeighbor;
            printf("Current: %s, Closest Neighbor: %s, Distance from Query: %f\n", current.name, closestNeighbor.name, euclideanDistance(closestNeighbor, q, 784));
        }

        if (results->size >= N) {
            break;  // Break out of the restart loop if enough neighbors are found
        }
    }

    sortListByDistance(results, q);
    truncateListToN(results, N);

    return results;
}


// Frees GNNS resources
void freeGNNS(GNNS* gnns) {
    if (gnns) {
        if (gnns->kNearestNeighborsGraph) {
            for (int i = 0; i < gnns->datasetSize; i++) {
                if (gnns->kNearestNeighborsGraph[i]) {
                    freeList(gnns->kNearestNeighborsGraph[i]);
                }
            }
            free(gnns->kNearestNeighborsGraph);
        }
        free(gnns);
    }
}


// Helper function to get the first E neighbors of a node from the k-NN graph
List* getNeighbors(List** kNearestNeighborsGraph, int nodeIndex, int E) {
    List* neighbors = initializeList();
    Node* currentNode = kNearestNeighborsGraph[nodeIndex]->head;
    int count = 0;
    while (currentNode != NULL && count < E) {
        insertToList(neighbors, currentNode->vector);
        currentNode = currentNode->next;
        count++;
    }
    return neighbors;
}

// Helper function to find the closest neighbor in a list to a query vector
Vector findClosestNeighbor(List* neighbors, Vector q) {
    Node* currentNode = neighbors->head;
    double minDistance = INFINITY;
    Vector closestNeighbor;

    while (currentNode != NULL) {
        double distance = euclideanDistance(currentNode->vector, q, 784);
        if (distance < minDistance) {
            minDistance = distance;
            closestNeighbor = currentNode->vector;
        }
        currentNode = currentNode->next;
    }
    return closestNeighbor;
}

// Helper function to check if a vector is in a list
int isInList(List* list, Vector vec) {
    Node* currentNode = list->head;
    while (currentNode != NULL) {
        if (strcmp(currentNode->vector.name, vec.name) == 0) {
            return 1; // Found
        }
        currentNode = currentNode->next;
    }
    return 0; // Not found
}

// Helper function to determine if a node is a local optimum
int isLocalOptimum(Vector current, List* neighbors, Vector q) {
    double currentDistance = euclideanDistance(current, q, 784);
    Node* currentNode = neighbors->head;
    while (currentNode != NULL) {
        if (euclideanDistance(currentNode->vector, q, 784) < currentDistance) {
            return 0; // Not a local optimum
        }
        currentNode = currentNode->next;
    }
    return 1; // Local optimum
}

// Helper function to truncate a list to N elements
void truncateListToN(List* list, int N) {
    if (list->size <= N) return;
    Node* currentNode = list->head;
    for (int i = 0; i < N - 1; i++) {
        currentNode = currentNode->next;
    }
    Node* temp;
    while (currentNode->next != NULL) {
        temp = currentNode->next;
        currentNode->next = temp->next;
        //free(temp);
    }
    list->size = N;
}
