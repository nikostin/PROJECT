// clustering/cluster.h

#ifndef CLUSTER_H
#define CLUSTER_H

#include "../vectors/vector.h"
#include "../lists/list.h"
#include "./update/macqueen.h"

typedef struct {
    Vector centroid;
    List* members;
} Cluster;


Cluster* initCluster(Vector initial_centroid);
void freeCluster(Cluster* cluster);
void addVectorToCluster(Cluster* cluster, Vector vec);
Cluster* findClusterByCentroid(List* clusters, Vector centroid);
int areVectorsEqual(Vector v1, Vector v2);
void insertClusterToList(List* list, Cluster* cluster);

#endif // CLUSTER_H
