#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>

#include "parse/parseMNIST.h"
#include "parse/convertMNIST.h"
#include "lists/list.h"
#include "search/lsh/lsh.h"
#include "search/hypercubes/hypercube.h" 
#include "search/distancemetrics/euclidean.h"
#include "search/bruteforce/exhaustive.h"


#define WINDOW_SIZE 0.5

// Function to obtain the file path from the user.
void getFilePath(char* path, char* dirPath, char* fullPath, const char* defaultFileName) {
    fgets(path, 256, stdin);

    // Remove the newline character that fgets adds
    size_t len = strlen(path);
    if (len > 0 && path[len-1] == '\n') {
        path[len-1] = '\0';
    }

    // Extract directory from path
    strcpy(dirPath, path);
    char *lastSlash = strrchr(dirPath, '/');
    if (lastSlash) {
        *(lastSlash + 1) = '\0';  // truncate the string after the last '/'
    } else {
        strcpy(dirPath, "./");  // default to current directory if no slash is found
    }

    sprintf(fullPath, "%s%s", dirPath, defaultFileName);

    // Check file extension
    char *ext = strrchr(path, '.');
    if (ext && strcmp(ext, ".idx3-ubyte") == 0) {
        // If this is a dataset
        if (strcmp(defaultFileName, "input.dat") == 0) {
            convertMNIST(path);  // Convert and create the corresponding .dat file
            strcpy(path, dirPath);
            strcat(path, "input.dat");  // Set path to the newly created input.dat
        }
        // If this is a query file
        else if (strcmp(defaultFileName, "query.dat") == 0) {
            createQueryDatFromIDX(path);  // Create query.dat directly from the given .idx3-ubyte file
            strcpy(path, dirPath);
            strcat(path, "query.dat");  // Set path to the newly created query.dat
        }
    }
}


// Function to prompt and get dataset path from user
void promptDatasetPath(char* datasetPath, char* dirPath, char* inputDatPath) {
    do {
        printf("\nEnter the path to your dataset (e.g. data/input.dat or data/train-images.idx3-ubyte): ");
        getFilePath(datasetPath, dirPath, inputDatPath, "input.dat");
        printf("Trying to access file at path: %s\n", inputDatPath);
    } while (access(inputDatPath, F_OK) == -1);
}

// Function to build search space with vectors
void buildSearchSpace(List* images, void* searchStructure, char* programName) {
    printf("Building search space with vectors...\n");
    int totalVectors = lengthOfList(images);
    int progressionBarLength = 50;
    int printEvery = totalVectors / progressionBarLength;
    if (printEvery == 0) printEvery = 1;

    Node* node = images->head;
    int count = 0;
    while (node) {
        if (strcmp(programName, "./lsh") == 0) {
            insertIntoLSH((LSH*)searchStructure, node->vector);
        } else if (strcmp(programName, "./cube") == 0) {
            insertIntoHypercube((Hypercube*)searchStructure, node->vector);
        }

        node = node->next;
        count++;
		//PROGRESS BARS
        if (count % printEvery == 0) {
            int filledChars = count / printEvery;
            printf("[");
            for (int i = 0; i < progressionBarLength; i++) {
                if (i < filledChars) {
                    printf("=");
                } else {
                    printf(" ");
                }
            }
            printf("] %.2f%%\r", (count * 100.0) / totalVectors);
            fflush(stdout);
        }
    }
    printf("\n");
    printf("Successfully inserted %d vectors into the search structure.\n", count);
}

// Function to prompt and get query path from user
void promptQueryPath(char* queryPath, char* dirPath, char* queryDatPath) {
    do {
        printf("\nEnter search file path (e.g. data/query.dat or data/t10k-images.idx3-ubyte): ");
        getFilePath(queryPath, dirPath, queryDatPath, "query.dat");
    } while (access(queryDatPath, F_OK) == -1);
}

// Function to prompt and get output path from user
FILE* promptOutputPath(char* outputPath) {
    printf("Enter output file path: ");
    fgets(outputPath, sizeof(outputPath), stdin);
    printf("you entered: %s", outputPath);
    FILE* outputFile = fopen(outputPath, "w");
    if (!outputFile) {
        printf("Error opening the output file for writing.\n");
        exit(1); // Terminating the program if unable to open the output file
    } else {
        printf("Output file successfully opened for writing.\n");
    }
    return outputFile;
}

// Function to process queries and write results to output
void processQueries(FILE* queryFile, void* searchStructure, FILE* outputFile, char* programName, int N, double R, int M, int probes, List* images) {

    // Read metadata from the query file
    int magicNumber, numOfQueryImages, rows, columns;
    fread(&magicNumber, sizeof(int), 1, queryFile);
    fread(&numOfQueryImages, sizeof(int), 1, queryFile);
    fread(&rows, sizeof(int), 1, queryFile);
    fread(&columns, sizeof(int), 1, queryFile);
    
    // Print the number of query images for debugging
    printf("Number of Query Images: %d\n", numOfQueryImages);

    // Process each query image
    for (int img = 0; img < numOfQueryImages; img++) {
        // Read image pixels from the query file
        unsigned char pixels[28][28];
        fread(&pixels, sizeof(unsigned char), 28 * 28, queryFile);
        
        // Initialize a vector from the image pixels
        Vector queryVector = initVectorFromImage(pixels, numOfQueryImages);

        // Variables to measure execution time of the search
        clock_t startSearch, endSearch;
        List* results = NULL;

        // Depending on the program name (LSH or Hypercube), perform the respective search
        if (strcmp(programName, "./lsh") == 0) {
            startSearch = clock();
            results = queryLSH((LSH*)searchStructure, queryVector, N, R);
            endSearch = clock();
        } else {
            startSearch = clock();
            results = queryHypercube((Hypercube*)searchStructure, queryVector, M, probes, N, R);
            endSearch = clock();
        }
        
        // Sort the search results by distance
        sortListByDistance(results, queryVector);
        
        // Calculate the elapsed time for the search
        double elapsedSearch = (double)(endSearch - startSearch) / CLOCKS_PER_SEC;

        // Perform an exhaustive search to find the true nearest neighbor
        clock_t startTrue = clock();
        Vector* trueNearestNeighbor = exhaustiveSearch(images, queryVector, N); // Memory allocated
        clock_t endTrue = clock();
        double elapsedTrue = (double)(endTrue - startTrue) / CLOCKS_PER_SEC;

        // Write results to the output file
        fprintf(outputFile, "Query: %d\n", img + 1);
        if (results && results->head) {
            fprintf(outputFile, "Nearest neighbor-1: %s %s: %f distanceTrue: %f\n", 
                    results->head->vector.name, 
                    (strcmp(programName, "./lsh") == 0) ? "distanceLSH" : "distanceHypercube", 
                    euclideanDistance(queryVector, results->head->vector, 784), 
                    euclideanDistance(queryVector, trueNearestNeighbor[0], 784));
        }

        Node* nthNode = getNthNode(results, N);
        if (nthNode && nthNode->vector.name) {
            fprintf(outputFile, "Nearest neighbor-N: %s %s: %f distanceTrue: %f %s: %f tTrue: %f\n", 
                    nthNode->vector.name, 
                    (strcmp(programName, "./lsh") == 0) ? "distanceLSH" : "distanceHypercube", 
                    euclideanDistance(queryVector, nthNode->vector, 784), 
                    euclideanDistance(queryVector, trueNearestNeighbor[N-1], 784), 
                    (strcmp(programName, "./lsh") == 0) ? "tLSH" : "tHypercube", 
                    elapsedSearch, 
                    elapsedTrue);
        }

        fprintf(outputFile, "R-near neighbors:");
        if (results) {
            Node* resultNode = results->head;
            while (resultNode) {
                fprintf(outputFile, " %s", resultNode->vector.name);
                resultNode = resultNode->next;
            }
        }
        fprintf(outputFile, "\n");
        
        // Free memory allocated for true nearest neighbors
        free(trueNearestNeighbor);
    }
}




int partA_main(int argc, char *argv[]) {
	
    // 1. Program Name Check:
    //////printf("[DEBUG] Program name: %s\n", argv[0]);

    char datasetPath[256] = "";
    char dirPath[256] = "";
    char inputDatPath[512] = "";
    char queryPath[256] = "";
    char queryDatPath[512] = "";
    char outputPath[256] = "";



	double w = WINDOW_SIZE;
    int k = 4, L = 5, N = 1, M = 10, probes = 2;
    double R = 10000.0;
	
	if (strcmp(argv[0], "./cube") == 0) {
		k =14;
	}

    void* searchStructure = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            strcpy(datasetPath, argv[++i]);
        } else if (strcmp(argv[i], "-q") == 0) {
            strcpy(queryPath, argv[++i]);
        } else if (strcmp(argv[i], "-o") == 0) {
            strcpy(outputPath, argv[++i]);
        } else if (strcmp(argv[i], "-k") == 0) {
            k = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-N") == 0) {
            N = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-R") == 0) {
            R = atof(argv[++i]);
        }
    }

    if (strcmp(argv[0], "./lsh") == 0) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-L") == 0) {
                L = atoi(argv[++i]);
            }
        }
        searchStructure = initLSH(k, L, N, R, w);
		
		
    } else if (strcmp(argv[0], "./cube") == 0) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-M") == 0) {
                M = atoi(argv[++i]);
            } else if (strcmp(argv[i], "-probes") == 0) {
                probes = atoi(argv[++i]);
            }
        }
		//printf("Value of k: %d\n", k);
		searchStructure = initHypercube(k, M, probes, N, R, w);
    }

    if (!searchStructure) {
        fprintf(stderr, "Error: searchStructure was not initialized.\n");
        return 1;
    }

    // 2. Initialization Function Returns:
    //////printf("[DEBUG] Value of searchStructure after initialization: %p\n", searchStructure);

//------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
    while (1) {
        if (strlen(datasetPath) == 0) {
            promptDatasetPath(datasetPath, dirPath, inputDatPath);
        } else {
            strcpy(inputDatPath, datasetPath);
        }

        List* images = readInputDat(inputDatPath);
        buildSearchSpace(images, searchStructure, argv[0]);

        if (strlen(queryPath) == 0) {
            promptQueryPath(queryPath, dirPath, queryDatPath);
        } else {
            strcpy(queryDatPath, queryPath);
        }

		FILE* outputFile;
		if (strlen(outputPath) == 0) {
			outputFile = promptOutputPath(outputPath);
		} else {
			outputFile = fopen(outputPath, "w");
			if (!outputFile) {
				printf("Error opening the output file for writing.\n");
				exit(1); // Terminating the program if unable to open the output file
			} else {
				printf("Output file successfully opened for writing.\n");
			}
		}




        FILE* queryFile = fopen(queryDatPath, "rb");
        if (queryFile) {
            processQueries(queryFile, searchStructure, outputFile, argv[0], N, R, M, probes, images);
            fclose(queryFile);
        } else {
            perror("Error opening the query file");
        }

        fclose(outputFile);
		printf("Successfully closed Output file.\n");
		
		
        char choice[10];
        do {
            printf("Do you want to terminate the program or repeat for a different dataset/search file? (terminate/repeat, or t/r): ");
            scanf("%9s", choice);
            while (getchar() != '\n');
        } while (strcmp(choice, "terminate") != 0 && strcmp(choice, "t") != 0 && 
                 strcmp(choice, "repeat") != 0 && strcmp(choice, "r") != 0);

        if (strcmp(choice, "terminate") == 0 || strcmp(choice, "t") == 0) {
            break;
        } else {
            memset(datasetPath, 0, sizeof(datasetPath));
            memset(queryPath, 0, sizeof(queryPath));
            memset(outputPath, 0, sizeof(outputPath));
		
	
    // Check if the program is running in hypercube mode
    if (strcmp(argv[0], "./cube") == 0) {
        // Free the memory and state of the old hypercube
        freeHypercube((Hypercube *)searchStructure);

        // Reinitialize the hypercube for the next iteration
        searchStructure = initHypercube(k, M, probes, N, R, w);
    }
        }
    }

    if (strcmp(argv[0], "./lsh") == 0) {
        freeLSH((LSH*)searchStructure);
    } else if (strcmp(argv[0], "./cube") == 0) {
        freeHypercube((Hypercube*)searchStructure);
    }

    return 0;
}