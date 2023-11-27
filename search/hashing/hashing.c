// /search/hashing/hashing.c

#include "hashing.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

HashFunction initHashFunction(int dimension, double window) {
    HashFunction hf;
    hf.v = (double *) malloc(dimension * sizeof(double));
    if (!hf.v) {
        perror("Memory allocation for hf.v failed");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < dimension; i++) {
        hf.v[i] = (rand() / (double)RAND_MAX) * 2 - 1; // Random values between -1 and 1
    }
	
	hf.w = window;
    hf.b = rand() / (double)RAND_MAX; // Random value between 0 and 1
    return hf;
}

void freeHashFunction(HashFunction* hf) {
    free(hf->v);
}

unsigned long hashVector(HashFunction* hf, Vector vec, int dimension) {
	// This is the h function which computes a hash value for a vector using a given hash function.

    double dotProduct = 0.0;
    for (int i = 0; i < dimension; i++) {
        dotProduct += hf->v[i] * vec.coordinates[i];
    }
	//return (unsigned long) floor(dotProduct + hf->b);
    return (dotProduct+ hf->b >= 0) ? 1 : 0;
}

HashTable initHashTable(int k, int dimension, double window) {
    HashTable ht;
    ht.k = k; 

    // Allocate memory for hash functions
    ht.hashFunctions = (HashFunction *) malloc(k * sizeof(HashFunction));
    if (!ht.hashFunctions) {
        perror("Memory allocation for ht.hashFunctions failed");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < k; i++) {
        ht.hashFunctions[i] = initHashFunction(dimension, window);
    }

    // Allocate memory for buckets
    ht.tableSize = 10007; 
    ht.buckets = (List **) calloc(ht.tableSize, sizeof(List *));
    if (!ht.buckets) {
        perror("Memory allocation for ht.buckets failed");
        for (int i = 0; i < k; i++) {
            freeHashFunction(&ht.hashFunctions[i]);
        }
        free(ht.hashFunctions);
        exit(EXIT_FAILURE);
    }

    return ht;
}


void insertIntoHashTable(HashTable* ht, Vector vec, int dimension) {
	    // This block represents the g function by combining k h function outputs.

    unsigned long combinedHash = 0;
    for (int i = 0; i < ht->k; i++) {
        combinedHash |= (hashVector(&ht->hashFunctions[i], vec, dimension) << i);
    }
    combinedHash %= ht->tableSize;

    if (!ht->buckets[combinedHash]) {
        ht->buckets[combinedHash] = initializeList();
    }
    insertToList(ht->buckets[combinedHash], vec);
}

List* queryHashTable(HashTable* ht, Vector q, int dimension, int N, double R) {
	    // This block represents the g function by combining k h function outputs.

    unsigned long combinedHash = 0;
    for (int i = 0; i < ht->k; i++) {
        combinedHash |= (hashVector(&ht->hashFunctions[i], q, dimension) << i);

    }
    combinedHash %= ht->tableSize;

    if (ht->buckets[combinedHash] && lengthOfList(ht->buckets[combinedHash]) > N) {
        List* truncatedList = initializeList();
        Node* current = ht->buckets[combinedHash]->head;
        int count = 0;
        while (current && count < N) {
            insertToList(truncatedList, current->vector);
            current = current->next;
            count++;
        }
        return truncatedList;
    }

    return ht->buckets[combinedHash];
}

void freeHashTable(HashTable* ht) {
    for (int i = 0; i < ht->k; i++) {
        freeHashFunction(&ht->hashFunctions[i]);
    }
    for (int i = 0; i < ht->tableSize; i++) {
        if (ht->buckets[i]) {
            freeList(ht->buckets[i]);
        }
    }
    free(ht->hashFunctions);
    free(ht->buckets);
}
