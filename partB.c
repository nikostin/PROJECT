#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <getopt.h>

#include "parse/parseMNIST.h"
#include "parse/convertMNIST.h"
#include "lists/list.h"
#include "vectors/vector.h"
#include "search/lsh/lsh.h"
#include "search/hypercubes/hypercube.h" 
#include "search/distancemetrics/euclidean.h"
#include "search/bruteforce/exhaustive.h"
#include "clustering/cluster.h"
#include "clustering/initialization/kmeanspp.h"
#include "clustering/update/macqueen.h"
#include "clustering/evaluation/silhouette.h" 
#include "clustering/assignment/lloyd.h" 
#include "clustering/assignment/lsh_reverse_search.h" 
#include "clustering/assignment/hypercube_reverse_search.h" 

#define MAX_ITERATIONS 100
#define WINDOW_SIZE 0.5
#define N_N 1
#define RADIUS_R 10000

typedef struct {
    int K;
    int L;
    int k_vec;
    int M;
    int k_hypercube;
    int probes;
} Config;


// Function prototypes for forward declarations
int areCentroidsEqual(List* list1, List* list2);
List* initializeCentroids(List* imagesList, int K);



/**
 * Reads the configuration file and populates the Config structure.
 * 
 * @param filePath: Path to the configuration file.
 * @param config: Pointer to a Config structure to populate.
 */
void readConfigFile(const char* filePath, Config* config) {
    // Attempt to open the configuration file for reading.
    FILE* file = fopen(filePath, "r");
    
    // If file opening failed, print an error message and exit.
    if (!file) {
        perror("Error opening configuration file");
        fprintf(stderr, "Failed to open configuration file: %s\n", filePath);
        exit(1);
    }

    // Initialize configuration structure with default values.
    config->L = 3;
    config->k_vec = 4;
    config->M = 10;
    config->k_hypercube = 3;
    config->probes = 2;

    char line[256];  // Buffer to read each line from the file.

    // Read the file line by line.
    while (fgets(line, sizeof(line), file)) {
        // Check each line for the expected configuration options.
        // Use strstr to identify the option and sscanf to extract the associated value.
        
        if (strstr(line, "number of clusters:") == line) {
            if (sscanf(line, "number of clusters: %d", &(config->K)) != 1) {
                fprintf(stderr, "Error parsing number of clusters\n");
            }
        } 
        else if (strstr(line, "number of vector hash tables:") == line) {
            if (sscanf(line, "number of vector hash tables: %d", &(config->L)) != 1) {
                fprintf(stderr, "Error parsing number of vector hash tables\n");
            }
        } 
        else if (strstr(line, "number of vector hash functions:") == line) {
            if (sscanf(line, "number of vector hash functions: %d", &(config->k_vec)) != 1) {
                fprintf(stderr, "Error parsing number of vector hash functions\n");
            }
        } 
        else if (strstr(line, "max number M hypercube:") == line) {
            if (sscanf(line, "max number M hypercube: %d", &(config->M)) != 1) {
                fprintf(stderr, "Error parsing max number M hypercube\n");
            }
        } 
        else if (strstr(line, "number of hypercube dimensions:") == line) {
            if (sscanf(line, "number of hypercube dimensions: %d", &(config->k_hypercube)) != 1) {
                fprintf(stderr, "Error parsing number of hypercube dimensions\n");
            }
        } 
        else if (strstr(line, "number of probes:") == line) {
            if (sscanf(line, "number of probes: %d", &(config->probes)) != 1) {
                fprintf(stderr, "Error parsing number of probes\n");
            }
        }
    }

    fclose(file);  // Close the file after reading.
}





// Function to assign vectors to the nearest cluster centroid.
void assignVectorsToClusters(List* data, List* centroids) {
    ClusteredNode* dataNode = (ClusteredNode*)data->head;
    int unassignedDataPoints = 0;
    while (dataNode) {
        double minDist = INFINITY;
        ClusteredNode* nearestCentroidNode = NULL;
        ClusteredNode* centroidNode = (ClusteredNode*)centroids->head;

        // Loop through each centroid to find the nearest one
        while (centroidNode) {
            double dist = euclideanDistance(dataNode->baseNode.vector, centroidNode->baseNode.vector, 784);
            if (dist < minDist) {
                minDist = dist;
                nearestCentroidNode = centroidNode;
            }
            centroidNode = (ClusteredNode*)centroidNode->baseNode.next;
        }

        // If a nearest centroid was found, assign it to the dataNode
        if (nearestCentroidNode) {
            dataNode->assignedCentroid = nearestCentroidNode;
           // printf("[DEBUG]Data point assigned to centroid: %s\n", nearestCentroidNode->baseNode.vector.name); 
        } else {
            printf("assignVectorsToClusters: Data point %s was not assigned to any cluster!\n", dataNode->baseNode.vector.name);
            unassignedDataPoints++;
        }

        // Move to the next data node
        dataNode = (ClusteredNode*)dataNode->baseNode.next;
    }

    if (unassignedDataPoints > 0) {
        printf("assignVectorsToClusters: %d data points were not assigned to any cluster!\n", unassignedDataPoints);
    }
}

// Function to check if two lists of centroids are equal
int areCentroidsEqual(List* list1, List* list2) {
    ClusteredNode* node1 = (ClusteredNode*)list1->head;
    ClusteredNode* node2 = (ClusteredNode*)list2->head;

	while (node1 && node2) {
		for (int i = 0; i < 784; i++) {
			if (node1->baseNode.vector.coordinates[i] != node2->baseNode.vector.coordinates[i]) {
				return 0;  // Vectors are not equal
			}
		}
		node1 = (ClusteredNode*)node1->baseNode.next;
		node2 = (ClusteredNode*)node2->baseNode.next;
	}

    // If one list is longer than the other
    if (node1 || node2) {
        return 0;
    }

    return 1;  // Lists are equal
}




List* initializeCentroids(List* imagesList, int K) {
    //printf("[DEBUG]Entering initializeCentroids with K = %d\n", K);
    printf("Initializing centroids using KMeans++...\n");
    List* result = kmeansPlusPlusInitialization(imagesList, K);
    
    // Assign unique names to each centroid
    Node* centroidNode = result->head;
    int centroidIndex = 0;
    while (centroidNode) {
        sprintf(centroidNode->vector.name, "Centroid%d", centroidIndex);
        centroidIndex++;
        centroidNode = centroidNode->next;
    }
    
    //printf("[DEBUG]Exiting initializeCentroids\n");
    return result;
}



void writeClustersToFile(List* centroids, List* data, const char* outputFilePath, double* silhouetteValues, int completeFlag, const char* method, double cputime) {
    FILE* file = fopen(outputFilePath, "w");
    if (!file) {
        perror("Error opening output file");
        return;
    }

    fprintf(file, "Algorithm: %s\n", method); 

    ClusteredNode* centroidNode = (ClusteredNode*)centroids->head;
    int clusterCounter = 0;


   // printf("[DEBUG] Starting to process each centroid for output file.\n");
    while (centroidNode) {
       // printf("[DEBUG] Processing Centroid %d\n", clusterCounter);
        int clusterSize = 0;
        ClusteredNode* dataNode = (ClusteredNode*)data->head;

      //  printf("[DEBUG] Starting to process each data node for Centroid %d\n", clusterCounter);
        while (dataNode) {
           // printf("[DEBUG] Checking Data Node: %p, Name: %s\n", dataNode, dataNode->baseNode.vector.name);

            if (dataNode->assignedCentroid == NULL) {
                printf("[ERROR] Assigned Centroid is NULL for Data Node: %s\n", dataNode->baseNode.vector.name);
                dataNode = (ClusteredNode*)dataNode->baseNode.next;
                continue;
            }

            //printf("[DEBUG] Assigned Centroid Address: %p, Name: %s\n", dataNode->assignedCentroid, dataNode->assignedCentroid->baseNode.vector.name);

            if (strcmp(dataNode->assignedCentroid->baseNode.vector.name, centroidNode->baseNode.vector.name) == 0) {
                clusterSize++;
                //printf("[DEBUG] Data Node assigned to Centroid %d: %s\n", clusterCounter, dataNode->baseNode.vector.name);
            }
            dataNode = (ClusteredNode*)dataNode->baseNode.next;
        }

       // printf("[DEBUG] Cluster Size for Centroid %d: %d\n", clusterCounter, clusterSize);

        fprintf(file, "CLUSTER-{%d} {size: %d, centroid: ", 1+ clusterCounter++, clusterSize);
        for (int i = 0; i < 784; i++) {
            fprintf(file, "%lf ", centroidNode->baseNode.vector.coordinates[i]);
        }
        fprintf(file, "}\n");

        centroidNode = (ClusteredNode*)centroidNode->baseNode.next;
    }
	fprintf(file, "\nclustering time: %lf\n", cputime);
    // Start the silhouette printout
	fprintf(file, "silhouette: [");

	// Print each cluster's silhouette score
	for (int i = 0; i < clusterCounter; i++) {
		fprintf(file, "%lf", silhouetteValues[i]);
		if (i < clusterCounter - 1) {
			fprintf(file, ", ");
		}
	}

	// Print the total average silhouette score
	fprintf(file, ", %lf]\n", silhouetteValues[clusterCounter]);

    if (completeFlag) {
        fprintf(file, "\n");
        centroidNode = (ClusteredNode*)centroids->head;
        clusterCounter = 0;
        while (centroidNode) {
            fprintf(file, "CLUSTER-{%d} {centroid, ", clusterCounter +1);
           // printf("[DEBUG] Detailed info for Centroid %d\n", clusterCounter);

            ClusteredNode* dataNode = (ClusteredNode*)data->head;
            while (dataNode) {
                if (strcmp(dataNode->assignedCentroid->baseNode.vector.name, centroidNode->baseNode.vector.name) == 0) {
                    fprintf(file, "%s, ", dataNode->baseNode.vector.name);
                    //printf("[DEBUG] Data Node in Detailed Info for Centroid %d: %s\n", clusterCounter, dataNode->baseNode.vector.name);
                }
                dataNode = (ClusteredNode*)dataNode->baseNode.next;
            }
            fprintf(file, "}\n");
            centroidNode = (ClusteredNode*)centroidNode->baseNode.next;
            clusterCounter++;
        }
    }

    fclose(file);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int partB_main(int argc, char *argv[]) {
    char inputFilePath[256] = "";
    char configFilePath[256] = "";
    char outputFilePath[256] = "";
    char method[20] = "";
    int completeFlag = 0;
	
	clock_t start, end;
	double cpu_time_used;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0) {
            strcpy(inputFilePath, argv[++i]);
        } else if (strcmp(argv[i], "-c") == 0) {
            strcpy(configFilePath, argv[++i]);
        } else if (strcmp(argv[i], "-o") == 0) {
            strcpy(outputFilePath, argv[++i]);
        } else if (strcmp(argv[i], "-complete") == 0) {
            completeFlag = 1;
        } else if (strcmp(argv[i], "-m") == 0) {
            strcpy(method, argv[++i]);
        }
    }

    // Validate mandatory parameters
    if (!inputFilePath[0] || !configFilePath[0] || !outputFilePath[0] || !method[0]) {
        fprintf(stderr, "Usage: %s -i <input file> -c <configuration file> -o <output file> -m <method: Classic OR LSH or Hypercube> [-complete]\n", argv[0]);
        return 1;
    }

    // Load the data and populate the config
    List* imagesList = readInputDat(inputFilePath);
    Config config;
    readConfigFile(configFilePath, &config);

    LSH* lsh = NULL;
    Hypercube* hypercube = NULL;

    // Initialize LSH or Hypercube based on the method
    if (strcmp(method, "LSH") == 0) {
        lsh = initLSH(config.k_vec, config.L, N_N, RADIUS_R,WINDOW_SIZE );
    } else if (strcmp(method, "Hypercube") == 0) {
        hypercube = initHypercube(config.k_hypercube, config.M, config.probes, N_N, RADIUS_R, WINDOW_SIZE );
    }

    // Initialize centroids using k-Means++
    List* centroids = initializeCentroids(imagesList, config.K);
    List* newCentroids = NULL;
    List* clusters = NULL;
    int iterations = 0;

    do {
        // Perform the assignment step based on the method
        if (strcmp(method, "Classic") == 0) {
			start = clock();
            assignWithLloyd(imagesList, centroids);
			end = clock();
        } else if (strcmp(method, "LSH") == 0) {
			start = clock();
            clusters = assignWithLSHReverseSearch(imagesList, centroids, lsh, N_N, RADIUS_R);
			end = clock();
			assignVectorsToClusters(imagesList, clusters); 
			freeList(clusters); // free the clusters list after assignment

        } else if (strcmp(method, "Hypercube") == 0) {
            // Implement Hypercube assignment logic
			start = clock();
			clusters = assignWithHypercubeReverseSearch(imagesList, centroids, hypercube,  N_N, RADIUS_R);
			end = clock();
			assignVectorsToClusters(imagesList, clusters); //  this function assigns from clusters list
			freeList(clusters); // free the clusters list after assignment
        } else {
            fprintf(stderr, "Invalid method specified. Choose from Classic, LSH, or Hypercube.\n");
            exit(1);
        }
		cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;


        // Update centroids using the MacQueen method
        newCentroids = macQueenUpdate(imagesList, centroids);

        if (iterations > 0) {
            freeList(centroids);
        }

        centroids = newCentroids;
        iterations++;

    } while (!areCentroidsEqual(centroids, newCentroids) && iterations < MAX_ITERATIONS);

    
    double* silhouetteValues = silhouetteScore(imagesList, centroids);

	
	// Calculate Silhouette scores:
    writeClustersToFile(centroids, imagesList, outputFilePath, silhouetteValues, completeFlag, method, cpu_time_used);
	free(silhouetteValues); 
	
	printf("Ouput file was written successfully! \n");
    return 0;
}