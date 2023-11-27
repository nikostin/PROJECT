// /search/distancemetrics/euclidean.c

#include "euclidean.h"
#include "../../lists/list.h"
#include <math.h>
#include <stdlib.h>
//#include <stdio.h>


Vector globalQueryVector;

double euclideanDistance(Vector v1, Vector v2, int dimension) {
    double sum = 0.0;
    for (int i = 0; i < dimension; i++) {
        sum += pow(v1.coordinates[i] - v2.coordinates[i], 2);
    }
    double result = sqrt(sum);
    //printf("euclideanDistance between %s and %s: %f\n", v1.name, v2.name, result);  // Debug print
    return result;
}

int compareVectorsByDistance(const void *a, const void *b) {
    double distA = euclideanDistance(((Node *)a)->vector, globalQueryVector, 784);
    double distB = euclideanDistance(((Node *)b)->vector, globalQueryVector, 784);
    if (distA < distB) return -1;
    if (distA > distB) return 1;
    return 0;
}

void sortListByDistance(List* list, Vector q) {
    globalQueryVector = q; // Set the global vector for comparison purposes

    // Dynamically allocate an array of nodes to hold the sorted list
    Node* array = malloc(list->size * sizeof(Node));
    
    // Populate the array with the nodes from the list
    Node* current = list->head;
    int i = 0;
    while (current) {
        array[i] = *current;
        current = current->next;
        i++;
    }

    // Sort the array based on distance
    qsort(array, list->size, sizeof(Node), compareVectorsByDistance);

    // Free the old nodes from the list as they will be replaced by the sorted array
    Node* oldCurrent = list->head;
    while (oldCurrent) {
        Node* oldNext = oldCurrent->next;
        free(oldCurrent);
        oldCurrent = oldNext;
    }

    // Update the list to point to the sorted array
    list->head = &array[0];
    list->tail = &array[list->size - 1];
    for (i = 0; i < list->size - 1; i++) {
        array[i].next = &array[i+1];
    }
    array[list->size - 1].next = NULL;
}

