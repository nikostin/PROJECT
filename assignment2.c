// /assignment2.c:

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "parse/parseMNIST.h"
#include "lists/list.h"
#include "search/lsh/lsh.h"
#include "search/gnns/gnns.h"
#include "search/mrng/mrng.h"
#include "search/bruteforce/exhaustive.h"
#include "search/distancemetrics/euclidean.h"

#define IMAGE_SIZE 28
#define DEFAULT_K_LSH 4
#define DEFAULT_L_LSH 5
#define DEFAULT_WINDOW_SIZE 0.5
#define DEFAULT_R_LSH 10000.0

#define DEFAULT_K_GNNS 50
#define DEFAULT_E 30
#define DEFAULT_R_GNNS 1
#define DEFAULT_N 1
#define DEFAULT_L_MRNG 20
#define DEFAULT_T 100
#define DEFAULT_RANDOM_RESTARTS 5

List* processInputFile(char *inputFilePath) {
    printf("Processing input file: %s\n", inputFilePath);
    return readInputDat(inputFilePath);
}

LSH* initLSHWithDataset(List* dataset, int k, int L, double R, double window) {
    LSH* lsh = initLSH(k, L, DEFAULT_N, R, window);
    Node* currentNode = dataset->head;
    while (currentNode) {
        insertIntoLSH(lsh, currentNode->vector);
       // printf("Inserted vector into LSH: %s\n", currentNode->vector.name);
        currentNode = currentNode->next;
    }
    return lsh;
}

List* processQueryFile(char *queryFilePath, int *numLoadedImages) {
    printf("Processing query file: %s\n", queryFilePath);
    FILE* file = fopen(queryFilePath, "rb");
    if (!file) {
        perror("Error opening query file");
        exit(EXIT_FAILURE);
    }

    List* queryImages = initializeList();
    unsigned char pixels[IMAGE_SIZE][IMAGE_SIZE];
    *numLoadedImages = 0;

    while (fread(pixels, sizeof(unsigned char), IMAGE_SIZE * IMAGE_SIZE, file) == IMAGE_SIZE * IMAGE_SIZE) {
        int imageNumber = *numLoadedImages + 1;  // Use an integer for image numbering
        Vector vec = initVectorFromImage(pixels, imageNumber);
        insertToList(queryImages, vec);
        (*numLoadedImages)++;

        printf("Image %d loaded into query set\n", imageNumber);
    }

    printf("Loaded %d query images.\n", *numLoadedImages);
    fclose(file);
    return queryImages;
}

void processOutput(char *outputFilePath, List* queryResults, List* dataset, Vector queryVector, int N, double avgApproxTime) {
    static FILE *outputFile = NULL;
    if (!outputFile) {
        outputFile = fopen(outputFilePath, "w");
        if (!outputFile) {
            perror("Error opening output file");
            exit(EXIT_FAILURE);
        }
        fprintf(outputFile, "GNNS Results\n");
    }

    fprintf(outputFile, "Query: %s\n", queryVector.name);

    // Start time for exhaustive search
    clock_t startExhaustive = clock();
    Vector* trueNearestNeighbors = exhaustiveSearch(dataset, queryVector, N);
    clock_t endExhaustive = clock();
    double timeExhaustive = ((double)(endExhaustive - startExhaustive)) / CLOCKS_PER_SEC;

    Node* resultNode = queryResults->head;
    double maxApproximationFactor = 0.0;

    for (int i = 0; i < N && resultNode != NULL; ++i) {
        if (strcmp(resultNode->vector.name, queryVector.name) != 0) {
            double distanceApproximate = euclideanDistance(queryVector, resultNode->vector, 784);
            double distanceTrue = euclideanDistance(queryVector, trueNearestNeighbors[i], 784);
            double approximationFactor = distanceApproximate / distanceTrue;
            if (approximationFactor > maxApproximationFactor) {
                maxApproximationFactor = approximationFactor;
            }

            fprintf(outputFile, "Nearest neighbor-%d: %s\n", i + 1, resultNode->vector.name);
            fprintf(outputFile, "distanceApproximate: %f\n", distanceApproximate);
            fprintf(outputFile, "distanceTrue: %f\n", distanceTrue);

            resultNode = resultNode->next;
        }
    }

    fprintf(outputFile, "tAverageApproximate: %f\n", avgApproxTime); 
    fprintf(outputFile, "tAverageTrue: %f\n", timeExhaustive);
    fprintf(outputFile, "MAF: %f\n\n", maxApproximationFactor);

    free(trueNearestNeighbors);
}
int assignment2_main(int argc, char *argv[]) {
    char *inputFilePath = NULL;
    char *queryFilePath = NULL;
    char *outputFilePath = NULL;
    int method = 0, k = DEFAULT_K_GNNS, N = DEFAULT_N, E = DEFAULT_E;
    double R_GNNS = DEFAULT_R_GNNS, R_LSH = DEFAULT_R_LSH;
    int l = DEFAULT_L_MRNG;
    int numQueryImages = 0;

    int opt;
    while ((opt = getopt(argc, argv, "d:q:o:m:k:E:R:N:l:")) != -1) {
        switch (opt) {
            case 'd': inputFilePath = optarg; break;
            case 'q': queryFilePath = optarg; break;
            case 'o': outputFilePath = optarg; break;
            case 'm': method = atoi(optarg); break;
            case 'k': k = atoi(optarg); break;
            case 'E': E = atoi(optarg); break;
            case 'R': R_GNNS = atof(optarg); break;
            case 'N': N = atoi(optarg); break;
            case 'l': l = atoi(optarg); break;
            default: 
                fprintf(stderr, "Usage: %s -d <input file> -q <query file> -o <output file> -m <method> -k <int> -E <int> -R <double for GNNS R> -N <int> -l <int>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (!inputFilePath || !queryFilePath || !outputFilePath || method <= 0 || method > 2) {
        fprintf(stderr, "Missing or invalid arguments.\n");
        exit(EXIT_FAILURE);
    }

    List* images = processInputFile(inputFilePath);
    printf("Number of images in dataset: %d\n", images->size);

    LSH* lsh = initLSHWithDataset(images, DEFAULT_K_LSH, DEFAULT_L_LSH, R_LSH, DEFAULT_WINDOW_SIZE);
    List* queryImages = processQueryFile(queryFilePath, &numQueryImages);

    GNNS* gnns = initGNNS(lsh, images, k, DEFAULT_T, DEFAULT_RANDOM_RESTARTS, E, R_GNNS);


    Node* queryNode = queryImages->head;
    while (queryNode) {
        Vector queryVector = queryNode->vector;
       // printf("Running GNNS for query image: %s\n", queryVector.name);

        clock_t startApprox = clock();
        List* queryResults = queryGNNS(gnns, queryVector, N);
        clock_t endApprox = clock();
        double avgApproxTime = ((double)(endApprox - startApprox)) / CLOCKS_PER_SEC / N;

        processOutput(outputFilePath, queryResults, images, queryVector, N, avgApproxTime);
        freeList(queryResults);
        queryNode = queryNode->next;
    }
	

    freeLSH(lsh);
    freeList(images);
    freeList(queryImages);
    freeGNNS(gnns);

    return 0;
}
