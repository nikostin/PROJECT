#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int32_t readInt(FILE *fp) {
    int32_t integer;
    fread(&integer, sizeof(integer), 1, fp);
    return __builtin_bswap32(integer); // Convert from Big Endian to Little Endian
}

void convertMNIST(const char *inputFileName) {
    FILE *mnistFile = fopen(inputFileName, "rb");
    if (!mnistFile) {
        perror("Cannot open MNIST file");
        return;
    }

    char outputFilePath[512];
    strcpy(outputFilePath, inputFileName);
    char *lastSlashOutput = strrchr(outputFilePath, '/');
    if (lastSlashOutput) {
        strcpy(lastSlashOutput + 1, "input.dat");
    } else {
        strcpy(outputFilePath, "input.dat");
    }

    FILE *outputFile = fopen(outputFilePath, "wb");
    if (!outputFile) {
        perror("Cannot open output file");
        return;
    }

    // Reading and Writing the Header
    int32_t magicNumber = readInt(mnistFile);
    int32_t originalMagicNumber = __builtin_bswap32(magicNumber);
    fwrite(&originalMagicNumber, sizeof(int32_t), 1, outputFile);

    int32_t numOfImages = readInt(mnistFile);
    int32_t originalNumOfImages = __builtin_bswap32(numOfImages);
    fwrite(&originalNumOfImages, sizeof(int32_t), 1, outputFile);

    int32_t rows = readInt(mnistFile);
    int32_t originalRows = __builtin_bswap32(rows);
    fwrite(&originalRows, sizeof(int32_t), 1, outputFile);

    int32_t cols = readInt(mnistFile);
    int32_t originalCols = __builtin_bswap32(cols);
    fwrite(&originalCols, sizeof(int32_t), 1, outputFile);

    // Reading and Writing the Pixels
    unsigned char pixel;
    for (int img = 0; img < numOfImages; img++) {
        for (int i = 0; i < rows * cols; i++) {
            fread(&pixel, sizeof(pixel), 1, mnistFile);
            fwrite(&pixel, sizeof(unsigned char), 1, outputFile);
        }
    }
    fclose(outputFile);
    fclose(mnistFile);
    printf("Conversion complete. Created 'input.dat' from the dataset.\n");
}

void createQueryDatFromIDX(const char* idxPath) {
    srand(time(NULL));

    char queryFilePath[512];
    strcpy(queryFilePath, idxPath);
	char *lastSlash = strrchr(queryFilePath, '/');
	if (lastSlash) {
		strcpy(lastSlash + 1, "query.dat");
	} else {
		strcpy(queryFilePath, "query.dat");
}
    FILE* idxFile = fopen(idxPath, "rb");
    if (!idxFile) {
        perror("Error opening the .idx3-ubyte file");
        return;
    }

    FILE* queryFile = fopen(queryFilePath, "wb");
    if (!queryFile) {
        perror("Error creating the query.dat file");
        fclose(idxFile);
        return;
    }

    int magicNumber = 2051;  // As per the MNIST dataset specification
    int numOfImagesToPick = (rand() % 10) + 1;  // Choose a number between 1 and 10 inclusive
    int rows = 28;
    int columns = 28;

    fwrite(&magicNumber, sizeof(int), 1, queryFile);
    fwrite(&numOfImagesToPick, sizeof(int), 1, queryFile);
    fwrite(&rows, sizeof(int), 1, queryFile);
    fwrite(&columns, sizeof(int), 1, queryFile);

    unsigned char pixels[28][28];

    // Create an array to hold indices of the first 10 images and shuffle it
    int indices[10];
    for (int i = 0; i < 10; i++) {
        indices[i] = i;
    }
    for (int i = 0; i < 10; i++) {
        int j = i + rand() / (RAND_MAX / (10 - i) + 1);
        int t = indices[j];
        indices[j] = indices[i];
        indices[i] = t;
    }

    // Pick and write the images based on the shuffled indices
    printf("Selected images %d from the first 10 images: ",numOfImagesToPick );
    for (int i = 0; i < numOfImagesToPick; i++) {
        fseek(idxFile, 16 + indices[i] * rows * columns, SEEK_SET); // Seek to the image's data
        fread(&pixels, sizeof(unsigned char), rows * columns, idxFile);
        fwrite(&pixels, sizeof(unsigned char), rows * columns, queryFile);
        printf("%d ", indices[i] + 1);  // Indices are 1-based for the user
    }
    printf("\n");

    fclose(idxFile);
    fclose(queryFile);
}
