// /parse/parseMNIST.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h> 
#include <string.h>  // Required for strerror


#include "parseMNIST.h"
#include "../vectors/vector.h"
#include "../lists/list.h"


Header readHeader(FILE* file) {
    Header header;
    printf("Reading header...\n");

    fread(&header.magicNumber, sizeof(int), 1, file);
    fread(&header.numOfImages, sizeof(int), 1, file);
    fread(&header.rows, sizeof(int), 1, file);
    fread(&header.columns, sizeof(int), 1, file);

    // Print out the raw values of the header components
    //printf("Raw Magic Number: %08x\n", header.magicNumber);
    //printf("Raw Number of Images: %08x\n", header.numOfImages);
    //printf("Raw Rows: %08x\n", header.rows);
    //printf("Raw Columns: %08x\n", header.columns);

    // Convert from big-endian to little-endian
    header.magicNumber = __builtin_bswap32(header.magicNumber);
    header.numOfImages = __builtin_bswap32(header.numOfImages);
    header.rows = __builtin_bswap32(header.rows);
    header.columns = __builtin_bswap32(header.columns);

    printf("Header read successfully. Magic Number: %d, Number of Images: %d, Rows: %d, Columns: %d\n", 
           header.magicNumber, header.numOfImages, header.rows, header.columns);

    return header;
}


Vector initVectorFromImage(unsigned char pixels[28][28], int imageNumber) {
    double vec[784];
    for (int i = 0; i < 28; i++) {
        for (int j = 0; j < 28; j++) {
            vec[i * 28 + j] = (double) pixels[i][j];
        }
    }
    char name[256]="";
    sprintf(name, "Image%d", imageNumber);
    return initVector(vec, name);
}

#define LOADER_LENGTH 50  // Length of the loader bar, e.g., 50 characters
//#define MAX_IMAGES 1000   // Maximum number of images to process

List* readImages(FILE* file, int numOfImages, int rows, int columns) {
    List* images = initializeList();
    int imagesToProcess = numOfImages;

    for (int i = 0; i < imagesToProcess; i++) {
        unsigned char pixels[28][28];
        fread(&pixels, sizeof(unsigned char), rows * columns, file);
		Vector vec = initVectorFromImage(pixels, i + 1);
        insertToList(images, vec);

        // Display the loader
        float percentage = (float) (i+1) / imagesToProcess;
        int position = (int) (LOADER_LENGTH * percentage);

        printf("\r[");
        for (int j = 0; j < LOADER_LENGTH; j++) {
            if (j < position) {
                printf("=");
            } else if (j == position) {
                printf(">");
            } else {
                printf(" ");
            }
        }
        printf("] %.2f%%", percentage * 100);
        fflush(stdout);
    }
    printf("\n Loaded images: %d \n", imagesToProcess);
    return images;
}



List* readInputDat(char* fileName) {
    //printf("Trying to access file at path: %s\n", fileName);

    FILE* file = fopen(fileName, "rb");
    if (file) {
        printf("File opened successfully\n");
        Header header = readHeader(file);
        printf("Header read complete. Moving to images...\n");
        List* images = readImages(file, header.numOfImages, header.rows, header.columns);
        printf("Reading images complete.\n");
        fclose(file);
        return images;
    } else {
        printf("Error opening the file %s: %s\n", fileName, strerror(errno));
        exit(EXIT_FAILURE); // Exit if file couldn't be opened
    }
}


// Clears the input buffer to remove any lingering characters.
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}