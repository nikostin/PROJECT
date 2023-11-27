// /search/distance/metrics/euclidean.h

#ifndef EUCLIDIAN_H
#define EUCLIDIAN_H

#include "../../vectors/vector.h" 
#include "../../lists/list.h"

double euclideanDistance(Vector v1, Vector v2, int dimension);

int compareVectorsByDistance(const void *a, const void *b);
void sortListByDistance(List* list, Vector q);

#endif
