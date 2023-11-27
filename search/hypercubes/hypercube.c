// /search/hypercubes/hypercube.c

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "hypercube.h"
#include "../distancemetrics/euclidean.h"
#include "../hashing/hashing.h"
#include "../../lists/list.h"



Hypercube* initHypercube(int k, int M, int probes, int N, double R, double window) {
    Hypercube* hc = (Hypercube *) malloc(sizeof(Hypercube));
    if (!hc) {
        perror("Failed to allocate memory for Hypercube");
        exit(EXIT_FAILURE);
    }

    // Initialize all fields to default values
    memset(hc, 0, sizeof(Hypercube));

    hc->k = k;
    hc->M = M;       // Set M
    hc->probes = probes; // Set probes
    hc->N = N;       // Set N
    hc->R = R;       // Set R
    hc->window = window;   // Set window

    hc->hashFunctions = (HashFunction *) malloc(k * sizeof(HashFunction));
    if (!hc->hashFunctions) {
        perror("Failed to allocate memory for hashFunctions");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < k; i++) {
        hc->hashFunctions[i] = initHashFunction(784, window); // 784 is the dimension for MNIST images
    }

    hc->buckets = (List **) calloc((1 << k), sizeof(List *));  // 2^k buckets
    if (!hc->buckets) {
        perror("Failed to allocate memory for buckets");
        exit(EXIT_FAILURE);
    }

    return hc;
}

void insertIntoHypercube(Hypercube* hc, Vector vec) {
    unsigned long bucketIndex = 0;
	    // This block represents the g function by combining k h function outputs.

    for (int i = 0; i < hc->k; i++) {
        unsigned long hashValue = hashVector(&hc->hashFunctions[i], vec, 784);
        // Ensuring hashValue is 0 or 1
        hashValue = hashValue & 1;
        bucketIndex |= (hashValue << i);
    }

    if (bucketIndex >= (1 << hc->k)) {
        fprintf(stderr, "Invalid bucket index detected: %lu\n", bucketIndex);
        return;
    }

    if (!hc->buckets[bucketIndex]) {
        hc->buckets[bucketIndex] = initializeList();
    }
    insertToList(hc->buckets[bucketIndex], vec);
}

List* queryHypercube(Hypercube* hc, Vector q, int M, int probes, int N, double R) {
	    // This block represents the g function by combining k h function outputs.
    List* result = initializeList();
    unsigned long bucketIndex = 0;
    int examinedCandidates = 0; // Initialize examined candidates count

    for (int i = 0; i < hc->k; i++) {
        unsigned long hashValue = hashVector(&hc->hashFunctions[i], q, 784);
        hashValue = hashValue & 1;
        bucketIndex |= (hashValue << i);
    }

    // Safety check
    if (bucketIndex >= (1 << hc->k)) {
        fprintf(stderr, "Invalid bucket index detected: %lu\n", bucketIndex);
        return result;
    }

    // Search in the exact bucket first
    if (hc->buckets[bucketIndex]) {
        Node* current = hc->buckets[bucketIndex]->head;
		while (current && result->size < hc->N && examinedCandidates < hc->M) {
            examinedCandidates++; // Increment the examined candidates count

			if (euclideanDistance(current->vector, q, 784) <= hc->R) {
                // Simple deduplication: Check if vector is already in result
                int found = 0;
                Node* resNode = result->head;
                while (resNode) {
                    if (strcmp(resNode->vector.name, current->vector.name) == 0) {
                        found = 1;
                        break;
                    }
                    resNode = resNode->next;
                }
                if (!found) {
                    insertToList(result, current->vector);
                }
            }
            current = current->next;
        }
    }

    // Probing in neighboring buckets
    for (int p = 0; p < hc->probes && p < hc->k && result->size < hc->N && examinedCandidates < hc->M; p++) {
        unsigned long probeIndex = bucketIndex ^ (1 << p); // Flip the p-th bit for neighboring bucket
        if (hc->buckets[probeIndex]) {
            Node* current = hc->buckets[probeIndex]->head;
			while (current && result->size < hc->N && examinedCandidates < hc->M) {
                examinedCandidates++; // Increment the examined candidates count

				if (euclideanDistance(current->vector, q, 784) <= hc->R) {
                    // Same deduplication process
                    int found = 0;
                    Node* resNode = result->head;
                    while (resNode) {
                        if (strcmp(resNode->vector.name, current->vector.name) == 0) {
                            found = 1;
                            break;
                        }
                        resNode = resNode->next;
                    }
                    if (!found) {
                        insertToList(result, current->vector);
                    }
                }
                current = current->next;
            }
        }
    }

    // Debug print
    //printf("Hypercube query returned %d vectors.\n", lengthOfList(result));
	//freeList(result); 
    return result;
}

void freeHypercube(Hypercube* hc) {
    for (int i = 0; i < (1 << hc->k); i++) {
		//freeHashFunction(&hc->hashFunctions[i]);
        if (hc->buckets[i]) {
            freeList(hc->buckets[i]);
        }
    }
    free(hc->hashFunctions);
    free(hc->buckets);
    free(hc);
}
