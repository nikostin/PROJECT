#ifndef MACQUEEN_H
#define MACQUEEN_H

#include "../../lists/list.h"
#include "../../vectors/vector.h"

typedef struct {
    Vector centroid;
    int count;
} ClusterNode;


List* macQueenUpdate(List* data, List* centroids);



#endif // MACQUEEN_H
