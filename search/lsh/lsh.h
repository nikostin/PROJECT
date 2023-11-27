// /search/lsh/lsh.h

#ifndef LSH_H
#define LSH_H

#include "../../lists/list.h"
#include "../../vectors/vector.h"
#include "../hashing/hashing.h"

typedef struct {
    int k;
    int L;
    int N;
    double R;
    HashTable* hashTables;
	double window;   // Window size
} LSH;

int getTotalVectorsInserted();
LSH* initLSH(int k, int L, int N, double R, double window);  // Updated function prototype
void insertIntoLSH(LSH* lsh, Vector vec);
List* queryLSH(LSH* lsh, Vector q, int N, double R);
void freeLSH(LSH* lsh);

#endif
