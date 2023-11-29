#include "mrng.h"
#include "../lsh/lsh.h"
#include "../distancemetrics/euclidean.h"
#include "../../lists/list.h"
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <string.h>

#define VECTOR_DIMENSION 784  // Define the dimension of the vectors

// Forward declarations of the functions used
Vector calculateDatasetCentroid(List* dataset);
List* findMRNGNeighbors(List* dataset, Vector* currentVector);
List* sortCandidates(List* dataset, Vector* currentVector);
int isMRNGNeighbor(List* neighbors, Vector* currentVector, Vector* candidate);
int compareVectorsByDistanceForMRNG(const void *a, const void *b);

// Global variable for the query vector used in sorting
static Vector globalQueryVector;

// Initialize MRNG structure with a dataset and LSH
MRNG* initMRNG(List* dataset, LSH* lsh, int l, int N, double R_lsh) {
    printf("Initializing MRNG...\n");
    // Check if the input parameters are valid
    if (!dataset || dataset->size == 0 || !lsh) {
        fprintf(stderr, "Invalid inputs for MRNG initialization\n");
        return NULL;
    }

    // Allocate memory for the MRNG structure
    MRNG* mrng = (MRNG*)malloc(sizeof(MRNG));
    if (!mrng) {
        perror("Failed to allocate memory for MRNG");
        exit(EXIT_FAILURE);
    }

    // Set the size of MRNG and allocate memory for its nodes
    mrng->size = dataset->size;
    mrng->nodes = (MRNGNode*)malloc(mrng->size * sizeof(MRNGNode));
    if (!mrng->nodes) {
        perror("Failed to allocate memory for MRNG nodes");
        free(mrng);
        exit(EXIT_FAILURE);
    }

    // Calculate the centroid of the dataset for navigating node
    mrng->navigatingNode = calculateDatasetCentroid(dataset);

    // Iterate through each node in the dataset
    Node* currentNode = dataset->head;
    for (int i = 0; i < mrng->size; i++) {
        mrng->nodes[i].vector = currentNode->vector;

        // Use LSH to find candidates for MRNG neighbors
        List* lshCandidates = queryLSH(lsh, currentNode->vector, l, R_lsh);
        mrng->nodes[i].neighbors = findMRNGNeighbors(lshCandidates, &currentNode->vector);

        // Free the list of candidates
        freeList(lshCandidates);
        currentNode = currentNode->next;
    }

    printf("MRNG initialized successfully.\n");
    return mrng;
}

// Find MRNG neighbors for a given vector in the dataset
List* findMRNGNeighbors(List* dataset, Vector* currentVector) {
    List* neighbors = initializeList();

    // Sort the candidates in the dataset based on their distance to the current vector
    List* sortedCandidates = sortCandidates(dataset, currentVector);

    // Get the minimal distance from the sorted list
    double minDistance = DBL_MAX;
    if (sortedCandidates->head != NULL) {
        minDistance = euclideanDistance(*currentVector, sortedCandidates->head->vector, VECTOR_DIMENSION);
    }

    // Add all candidates with the minimal distance to the neighbors list
    Node* candidateNode = sortedCandidates->head;
    while (candidateNode != NULL && euclideanDistance(*currentVector, candidateNode->vector, VECTOR_DIMENSION) <= minDistance) {
        insertToList(neighbors, candidateNode->vector);
        candidateNode = candidateNode->next;
    }

    // Free the sorted candidates list
    freeList(sortedCandidates);
    return neighbors;
}

// Sort candidates for MRNG
void sortCandidatesForMRNG(List* list, Vector q) {
    if (!list || list->size == 0) return;

    // Allocate an array of Node pointers for sorting
    Node** array = malloc(list->size * sizeof(Node*));
    if (!array) {
        perror("Failed to allocate memory for sorting array");
        exit(EXIT_FAILURE);
    }

    // Populate the array with pointers to the nodes of the list
    Node* current = list->head;
    for (int i = 0; i < list->size; i++) {
        array[i] = current;
        current = current->next;
    }

    // Set the global query vector for comparison
    globalQueryVector = q;

    // Use standard qsort function to sort the array of Node pointers
    qsort(array, list->size, sizeof(Node*), compareVectorsByDistanceForMRNG);

    // Re-link the nodes in the original list according to the sorted array
    list->head = array[0];
    list->head->next = array[1];
    for (int i = 1; i < list->size - 1; i++) {
        array[i]->next = array[i + 1];
    }
    array[list->size - 1]->next = NULL;
    list->tail = array[list->size - 1];

    // Free the sorting array
    free(array);
}

// Sort candidates for a given current vector
List* sortCandidates(List* dataset, Vector* currentVector) {
    //printf("Sorting candidates...\n");
    List* candidates = initializeList();

    // Iterate through the dataset and add all vectors except the current one
    Node* currentNode = dataset->head;
    while (currentNode != NULL) {
        if (strcmp(currentNode->vector.name, currentVector->name) != 0) {
            insertToList(candidates, currentNode->vector);
        }
        currentNode = currentNode->next;
    }

    // Sort the candidates using a custom function
    sortCandidatesForMRNG(candidates, *currentVector);
    //printf("Candidates sorted.\n");
    return candidates;
}

// Check if a candidate vector is an MRNG neighbor
int isMRNGNeighbor(List* neighbors, Vector* currentVector, Vector* candidate) {
    Node* neighborNode = neighbors->head;
    while (neighborNode != NULL) {
        if (neighborNode->vector.name != candidate->name) {
            double neighborDistance = euclideanDistance(*currentVector, neighborNode->vector, VECTOR_DIMENSION);
            double triangleDistance = euclideanDistance(*candidate, neighborNode->vector, VECTOR_DIMENSION);
            if (triangleDistance < neighborDistance) {
                return 0; // Candidate is not a MRNG neighbor
            }
        }
        neighborNode = neighborNode->next;
    }
    return 1; // Candidate is a MRNG neighbor
}

// Query MRNG for nearest neighbors
List* queryMRNG(MRNG* mrng, Vector query, int N) {
    printf("Querying MRNG for query vector: %s\n", query.name);
    List* result = initializeList();
    if (!mrng || N <= 0) {
        printf("MRNG structure not initialized or N is non-positive.\n");
        return result;
    }

    // Start from a random node in MRNG
    int startIndex = rand() % mrng->size;
    Vector currentVector = mrng->nodes[startIndex].vector;
    printf("Starting from random node index %d, vector name: %s\n", startIndex, currentVector.name);

    // Iterate N times to find nearest neighbors
    for (int i = 0; i < N; i++) {
        printf("Iteration %d\n", i+1);
        Node* neighborNode = mrng->nodes[startIndex].neighbors->head;
        if (!neighborNode) {
            printf("No neighbors found for node index %d\n", startIndex);
            break;
        }

        Vector closestVector = currentVector;
        double closestDistance = euclideanDistance(query, currentVector, VECTOR_DIMENSION);
        printf("Current closest distance: %f\n", closestDistance);

        // Iterate through neighbors to find the closest one
        while (neighborNode) {
            double dist = euclideanDistance(query, neighborNode->vector, VECTOR_DIMENSION);
            printf("Checking neighbor %s, distance to query: %f\n", neighborNode->vector.name, dist);

            if (dist < closestDistance) {
                closestVector = neighborNode->vector;
                closestDistance = dist;
                printf("New closest neighbor found: %s, distance: %f\n", closestVector.name, closestDistance);
            }
            neighborNode = neighborNode->next;
        }

        // Add closest neighbor to the result list
        if (closestVector.name != currentVector.name) {
            insertToList(result, closestVector);
            printf("Adding closest neighbor to result: %s\n", closestVector.name);
            currentVector = closestVector;
        } else {
            printf("No closer neighbor found, stopping iteration.\n");
            break; // Break if no closer neighbor is found
        }
    }
    printf("MRNG query completed. Number of results: %d\n", result->size);
    return result;
}

// Calculate the centroid of a dataset
Vector calculateDatasetCentroid(List* dataset) {
    printf("Calculating dataset centroid...\n");
    Vector centroid = {0};

    // Sum the coordinates of all vectors in the dataset
    Node* currentNode = dataset->head;
    while (currentNode) {
        for (int i = 0; i < VECTOR_DIMENSION; ++i) {
            centroid.coordinates[i] += currentNode->vector.coordinates[i];
        }
        currentNode = currentNode->next;
    }

    // Divide each coordinate by the number of vectors
    for (int i = 0; i < VECTOR_DIMENSION; ++i) {
        centroid.coordinates[i] /= dataset->size;
    }
    printf("Centroid calculated.\n");
    return centroid;
}

// Comparison function for sorting vectors by their distance to a global query vector
int compareVectorsByDistanceForMRNG(const void *a, const void *b) {
    Node* nodeA = *(Node**)a;
    Node* nodeB = *(Node**)b;
    double distA = euclideanDistance(nodeA->vector, globalQueryVector, VECTOR_DIMENSION);
    double distB = euclideanDistance(nodeB->vector, globalQueryVector, VECTOR_DIMENSION);
    return (distA > distB) - (distA < distB);
}


// Free resources allocated for MRNG
void freeMRNG(MRNG* mrng) {
    printf("Freeing MRNG resources...\n");
    if (!mrng) {
        return;
    }

    // Free each list of neighbors in the MRNG nodes
    for (int i = 0; i < mrng->size; i++) {
        freeList(mrng->nodes[i].neighbors);
    }

    // Free the nodes array and the MRNG structure itself
    free(mrng->nodes);
    free(mrng);
    printf("MRNG resources freed.\n");
}
