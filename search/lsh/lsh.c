// /search/lsh/lsh.c

#include "lsh.h"
#include "../hashing/hashing.h"
#include "../distancemetrics/euclidean.h"
#include "../../lists/list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


//typedef struct SearchResult {
//    List* nearestNeighbors;
//    List* rNearNeighbors;
//} SearchResult;


static int totalVectorsInserted = 0; // This will keep count across multiple calls

LSH* initLSH(int k, int L, int N, double R, double window) {
    LSH* lsh = (LSH *) malloc(sizeof(LSH));
    if (!lsh) {
        perror("Failed to allocate memory for LSH");
        exit(EXIT_FAILURE);
    }

    // Initialize all fields to default values
    memset(lsh, 0, sizeof(LSH));

    lsh->k = k;
    lsh->L = L;
    lsh->N = N;   // Set N
    lsh->R = R;   // Set R
    lsh->window = window; // Set w
    lsh->hashTables = (HashTable *) malloc(L * sizeof(HashTable));
    
    if (!lsh->hashTables) {
        perror("Failed to allocate memory for hashTables");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < L; i++) {
        lsh->hashTables[i] = initHashTable(k, 784, window); // 784 is the dimension for MNIST images
    }
    return lsh;
}



void insertIntoLSH(LSH* lsh, Vector vec) {
	    // This block represents the g function by combining k h function outputs.

    for (int i = 0; i < lsh->L; i++) {
        insertIntoHashTable(&lsh->hashTables[i], vec, 784);
    }
    totalVectorsInserted++;
}

List* queryLSH(LSH* lsh, Vector q, int N, double R) {
	    // This block represents the g function by combining k h function outputs.

    //List* nearestNeighbors = initializeList();
   // List* rNearNeighbors = initializeList();
    
    List* result = initializeList();

	for (int i = 0; i < lsh->L; i++) {

        List* bucket = queryHashTable(&lsh->hashTables[i], q, 784, lsh->N, lsh->R);

		
		if (!bucket) {
			continue;
		}
        
        if (bucket) {
			Node* current = bucket->head;
			while (current) {
				//if (!current->vector.name) {
					////////printf("[DEBUG]Vector name is NULL!\n");
					//continue;
					//}
				if (euclideanDistance(current->vector, q, 784) <= lsh->R) {
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
                        ////////printf("[DEBUG]Inserting vector to result\n");
                        insertToList(result, current->vector);
                        ////////printf("[DEBUG]Inserted vector to result\n");
                    }
                }
                current = current->next;
            }
        }
        
        // Debug print
        //printf("LSH table %d returned %d vectors.\n", i, lengthOfList(bucket));
    }
    ////////printf("[DEBUG]End of queryLSH\n");
	//freeList(result); 
    return result;
}

void freeLSH(LSH* lsh) {
    for (int i = 0; i < lsh->L; i++) {
        freeHashTable(&lsh->hashTables[i]);
    }
    free(lsh->hashTables);
    free(lsh);
}
