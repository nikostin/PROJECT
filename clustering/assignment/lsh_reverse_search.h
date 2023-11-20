#ifndef LSH_REVERSE_SEARCH_H
#define LSH_REVERSE_SEARCH_H

#include "../../lists/list.h"
#include "../../search/lsh/lsh.h"
#include "../cluster.h"


// Function to assign data points to clusters using LSH reverse search

List* assignWithLSHReverseSearch(List* data, List* centroids, LSH* lsh, int N, double R);

#endif // LSH_REVERSE_SEARCH_H
