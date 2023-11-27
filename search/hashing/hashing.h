// /search/hashing/hashing.h

#ifndef HASHING_H
#define HASHING_H

#include "../../lists/list.h" 
#include "../../vectors/vector.h" 

typedef struct {
    double* v;       // Random vector for hashing
    double b;        // Random shift for hashing
	double w;  // Window size
} HashFunction;

typedef struct {
    int k;                      // Number of hash functions
    HashFunction* hashFunctions; // Array of hash functions
    int tableSize;               // Size of the hash table
    List** buckets;              // Array of pointers to lists (buckets)
} HashTable;

// Function declarations for hashing.c
HashFunction initHashFunction(int dimension, double window);
unsigned long hashVector(HashFunction* hf, Vector vec, int dimension);
HashTable initHashTable(int k, int dimension, double window);
void insertIntoHashTable(HashTable* ht, Vector vec, int dimension);
List* queryHashTable(HashTable* ht, Vector q, int dimension, int N, double R);
void freeHashTable(HashTable* ht);
void freeHashFunction(HashFunction* hf);

#endif // HASHING_H
