// clustering/cluster.c

#include "cluster.h"
#include <stdlib.h>
#include <stdio.h>

Cluster* initCluster(Vector initial_centroid) {
    Cluster* newCluster = (Cluster*)malloc(sizeof(Cluster));
    if (!newCluster) {
        perror("Failed to allocate memory for new cluster");
        exit(EXIT_FAILURE);
    }

    newCluster->centroid = initial_centroid;
    newCluster->members = initializeList(); // initializeList initializes a new List

    if (!newCluster->members) {
        perror("Failed to allocate memory for cluster members list");
        free(newCluster); // Avoid memory leak
        exit(EXIT_FAILURE);
    }

    return newCluster;
}

void freeCluster(Cluster* cluster) {
    if (!cluster) return;

    // Free members list and its content
    freeList(cluster->members); // freeList takes care of freeing the list and its nodes
    free(cluster);
}

void addVectorToCluster(Cluster* cluster, Vector vec) {
    if (cluster && cluster->members) {
        insertToList(cluster->members, vec); // insertToList adds a vector to the list
    }
}

Cluster* findClusterByCentroid(List* clusters, Vector centroid) {
    Node* current = clusters->head;
    while (current) {
        Cluster* cluster = (Cluster*)current; // Cast the Node to Cluster
        if (areVectorsEqual(cluster->centroid, centroid)) {
            return cluster;
        }
        current = current->next;
    }
    return NULL;
}

int areVectorsEqual(Vector v1, Vector v2) {
    for (int i = 0; i < 784; i++) {
        if (v1.coordinates[i] != v2.coordinates[i]) {
            return 0; // Vectors are not equal
        }
    }
    return 1; // Vectors are equal
}

void insertClusterToList(List* list, Cluster* cluster) {
    if (!list || !cluster) return;

    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        perror("Failed to allocate memory for new node");
        exit(EXIT_FAILURE);
    }
    newNode->vector = cluster->centroid; // Insert the centroid as the vector in the list
    newNode->next = NULL;

    if (!list->head) {
        list->head = newNode;
        list->tail = newNode;
    } else {
        list->tail->next = newNode;
        list->tail = newNode;
    }
    list->size++;
}
