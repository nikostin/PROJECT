#include "mrng.h"
#include "../distancemetrics/euclidean.h"
#include "../lsh/lsh.h"
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <string.h>

#define VECTOR_DIMENSION 784  // Dimension of the vectors


// Function prototypes
void sortCandidates(List* dataset, Vector currentVector);
MRNGNode* findInitialNode(MRNG* mrng, Vector queryVector);
int vectorExistsInList(List* list, Vector vector);
MRNGNode* getMRNGNode(MRNG* mrng, Vector vector);
void freeMRNG(MRNG* mrng);
List* copyList(List* original);
void sortMRNGNeighborsByDistance(List* neighbors);


// initMRNG function
MRNG* initMRNG(List* dataset, LSH* lsh, int l, int N, double R_lsh) {
    printf("initMRNG in\n");
    if (!dataset || dataset->size == 0 || !lsh) {
        fprintf(stderr, "Invalid inputs for MRNG initialization\n");
        return NULL;
    }

    MRNG* mrng = (MRNG*)malloc(sizeof(MRNG));
    if (!mrng) {
        perror("Failed to allocate memory for MRNG");
        exit(EXIT_FAILURE);
    }

    mrng->size = dataset->size;
    mrng->nodes = (MRNGNode*)malloc(mrng->size * sizeof(MRNGNode));
    if (!mrng->nodes) {
        perror("Failed to allocate memory for MRNG nodes");
        free(mrng);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < mrng->size; i++) {
       // printf("Processing node %d\n", i);
        mrng->nodes[i].vector = *getVectorAt(dataset, i);

        // Use LSH to get preliminary neighbors
        List* lshCandidates = queryLSH(lsh, mrng->nodes[i].vector, l, R_lsh);

        // Sort LSH candidates
        globalQueryVector = mrng->nodes[i].vector;
        sortListByDistance(lshCandidates, globalQueryVector);

        // Initialize Lp (neighbor set) for each node
        List* Lp = initializeList();

        // Iterate through sorted LSH candidates
        Node* candidate = lshCandidates->head;
        while (candidate) {
            Vector r = candidate->vector;

            // Check for the monotonic path condition
            int condition = 1;
            for (Node* t = Lp->head; t != NULL; t = t->next) {
                if (euclideanDistance(t->vector, r, VECTOR_DIMENSION) <
                    euclideanDistance(t->vector, mrng->nodes[i].vector, VECTOR_DIMENSION)) {
                    condition = 0; // Not a monotonic path
                    break;
                }
            }

            // If the condition holds true, add r as a neighbor of p
            if (condition) {
                insertToList(Lp, r);
            }

            candidate = candidate->next;
        }

        // Assign the neighbor set to the current MRNG node
        mrng->nodes[i].neighbors = Lp;

        // Clean up
        freeList(lshCandidates);
    }

    printf("initMRNG out\n");
    return mrng;
}

// Helper function to create a dynamic array of vectors excluding the current vector
Vector* createDatasetArray(List* dataset, Vector currentVector) {
	//printf("createDatasetArray in\n");
    Vector* array = (Vector*)malloc((dataset->size - 1) * sizeof(Vector));
    int index = 0;
    for (Node* node = dataset->head; node != NULL; node = node->next) {
        if (strcmp(node->vector.name, currentVector.name) != 0) {
            array[index++] = node->vector;
        }
    }
	//printf("createDatasetArray out\n");
    return array;
}

List* copyList(List* original) {
	printf("CopyList in\n");
    if (original == NULL) {
        return NULL;
    }

    List* newList = initializeList();
    if (!newList) {
        perror("Failed to allocate memory for new list copy");
        exit(EXIT_FAILURE);
    }

    Node* current = original->head;
    while (current != NULL) {
        // Copy the vector
        Vector vectorCopy = current->vector; // Assuming a shallow copy is sufficient for the Vector

        // Insert the copied vector into the new list
        insertToList(newList, vectorCopy);

        current = current->next;
    }
	printf("CopyList out\n");
    return newList;
}


void sortCandidates(List* dataset, Vector currentVector) {
	//printf("sortCandidates in\n");
    if (!dataset || dataset->size <= 1) {
        return;
    }

    sortListByDistance(dataset, currentVector);
	//printf("sortCandidates out\n");
}

int vectorExistsInList(List* list, Vector vector) {
	//printf("vectorExistsInList in\n");
    Node* current = list->head;
    while (current) {
        if (strcmp(current->vector.name, vector.name) == 0) {
            return 1; // Vector found
        }
        current = current->next;
    }
	//printf("vectorExistsInList out\n");
    return 0; // Vector not found
}

// queryMRNG function
List* queryMRNG(MRNG* mrng, Vector queryVector, int N) {
    //printf("queryMRNG in\n");
    if (!mrng || !mrng->nodes || N <= 0) {
        fprintf(stderr, "Invalid inputs for MRNG query\n");
        return NULL;
    }

    List* nearestNeighbors = initializeList();
    //printf("Finding initial node for the query vector\n");
    MRNGNode* currentNode = findInitialNode(mrng, queryVector);
   // printf("Initial node found\n");

    List* visited = initializeList();
while (currentNode && nearestNeighbors->size < N) {
    printf("Processing node %s\n", currentNode->vector.name);

    if (vectorExistsInList(visited, currentNode->vector)) {
        printf("Node %s already visited\n", currentNode->vector.name);
    } else {
        // Process the unvisited node
        insertToList(visited, currentNode->vector);
        if (nearestNeighbors->size < N || 
            euclideanDistance(currentNode->vector, queryVector, VECTOR_DIMENSION) <
            euclideanDistance(nearestNeighbors->tail->vector, queryVector, VECTOR_DIMENSION)) {
            insertToList(nearestNeighbors, currentNode->vector);
        }
    }

    // Sort the neighbors of the current node based on their distance to the query vector
    sortMRNGNeighborsByDistance(currentNode->neighbors);

    // Move to the closest unvisited neighbor
    Node* nextNeighbor = currentNode->neighbors->head;
    while (nextNeighbor && vectorExistsInList(visited, nextNeighbor->vector)) {
        nextNeighbor = nextNeighbor->next; // Skip visited neighbors
    }

    if (nextNeighbor) {
        currentNode = getMRNGNode(mrng, nextNeighbor->vector); // Move to the next unvisited neighbor
    } else {
        break; // No more unvisited neighbors to process
    }
}

    freeList(visited);
    //printf("queryMRNG out\n");
    return nearestNeighbors;
}

MRNGNode* getMRNGNode(MRNG* mrng, Vector vector) {
   // printf("getMRNGNode in\n");
    for (int i = 0; i < mrng->size; i++) {
        if (strcmp(mrng->nodes[i].vector.name, vector.name) == 0) {
           // printf("Node found for vector %s\n", vector.name);
            return &mrng->nodes[i];
        }
    }
   // printf("Node not found for vector %s\n", vector.name);
    return NULL;
}

MRNGNode* findInitialNode(MRNG* mrng, Vector queryVector) {
   // printf("findInitialNode in\n");
    double minDistance = DBL_MAX;
    MRNGNode* initialNode = NULL;

    for (int i = 0; i < mrng->size; i++) {
        double distance = euclideanDistance(mrng->nodes[i].vector, queryVector, VECTOR_DIMENSION);
        if (distance < minDistance) {
            minDistance = distance;
            initialNode = &mrng->nodes[i];
        }
    }
   // printf("findInitialNode out\n");
    return initialNode;
}
// Free MRNG structure
void freeMRNG(MRNG* mrng) {
    if (mrng) {
        for (int i = 0; i < mrng->size; i++) {
            if (mrng->nodes[i].neighbors) {
                freeList(mrng->nodes[i].neighbors);
                mrng->nodes[i].neighbors = NULL;
            }
        }
        free(mrng->nodes);
        mrng->nodes = NULL;
        free(mrng);
        mrng = NULL;
    }
}

// Compare two vectors based on their Euclidean distance to a global query vector
int compareMRNGVectorsByDistance(const void *a, const void *b) {
    Vector v1 = ((Node *)a)->vector;
    Vector v2 = ((Node *)b)->vector;
    double distA = euclideanDistance(v1, globalQueryVector, VECTOR_DIMENSION);
    double distB = euclideanDistance(v2, globalQueryVector, VECTOR_DIMENSION);
    if (distA < distB) return -1;
    if (distA > distB) return 1;
    return 0;
}

// Function to sort the neighbors of a node in MRNG
void sortMRNGNeighborsByDistance(List* neighbors) {
    if (!neighbors || neighbors->size <= 1) {
        return; // No need to sort if the list is empty or has only one element
    }

    // Convert the linked list to an array for sorting
    Node* nodesArray = malloc(neighbors->size * sizeof(Node));
    if (!nodesArray) {
        perror("Failed to allocate memory for nodes array");
        exit(EXIT_FAILURE);
    }

    Node* current = neighbors->head;
    for (int i = 0; current != NULL; i++) {
        nodesArray[i] = *current; // Copy node data to the array
        current = current->next;
    }

    // Sort the array
    qsort(nodesArray, neighbors->size, sizeof(Node), compareMRNGVectorsByDistance);

    // Re-link the nodes in the list according to the sorted array
    neighbors->head = &nodesArray[0];
    neighbors->tail = &nodesArray[neighbors->size - 1];
    for (int i = 0; i < neighbors->size - 1; i++) {
        nodesArray[i].next = &nodesArray[i + 1];
    }
    nodesArray[neighbors->size - 1].next = NULL;

    // Free the original nodes in the list, if necessary
    
}