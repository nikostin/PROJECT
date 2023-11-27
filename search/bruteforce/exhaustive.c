// /search/bruteforce/exhaustive.c

#include "exhaustive.h"
#include "../distancemetrics/euclidean.h"
#include <math.h>
#include <stdlib.h>
//#include <stdio.h>  // Added for printf

typedef struct {
    Vector vector;
    double distance;
} DistancePair;

Vector* exhaustiveSearch(List* images, Vector queryVector, int N) {
    DistancePair closest[N];
    for (int i = 0; i < N; i++) {
        closest[i].distance = INFINITY;  // Initialize with infinity
    }

    Node* node = images->head;
    while (node) {
        double currentDistance = euclideanDistance(queryVector, node->vector, 784);
        for (int i = 0; i < N; i++) {
            if (currentDistance < closest[i].distance) {
                // Shift the rest of the elements down
                for (int j = N - 1; j > i; j--) {
                    closest[j] = closest[j - 1];
                }
                closest[i].distance = currentDistance;
                closest[i].vector = node->vector;
                break;
            }
        }
        node = node->next;
    }

    // Print the distances of the closest N vectors
  //  for(int i = 0; i < N; i++) {
  //      printf("Exhaustive - Closest %d: %s, Distance: %lf\n", i+1, closest[i].vector.name, closest[i].distance);
   // }

    // Allocate memory for the result array and fill it with the closest N vectors
    Vector* result = (Vector*) malloc(N * sizeof(Vector));
    for(int i = 0; i < N; i++) {
        result[i] = closest[i].vector;
    }

    return result;
}
