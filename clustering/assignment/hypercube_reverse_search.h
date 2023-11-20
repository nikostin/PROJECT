#ifndef HYPERCUBE_REVERSE_SEARCH_H
#define HYPERCUBE_REVERSE_SEARCH_H

#include "../../lists/list.h"
#include "../../search/hypercubes/hypercube.h"
#include "../cluster.h"  

// Function to assign data points to clusters using Hypercube reverse search
List* assignWithHypercubeReverseSearch(List* data, List* centroids, Hypercube* hypercube, int N, double R);

#endif // HYPERCUBE_REVERSE_SEARCH_H
