// /parse/parseMNIST.h
#ifndef PARSING_MNIST_H
#define PARSING_MNIST_H
#include "../lists/list.h"
#include "../vectors/vector.h"

typedef struct {
    int magicNumber;
    int numOfImages;
    int rows;
    int columns;
} Header;

Vector initVectorFromImage(unsigned char pixels[28][28], int imageNumber);
Header readHeader(FILE* file);
List* readImages(FILE* file, int numOfImages, int rows, int columns);
List* readInputDat(char* fileName);
void clearInputBuffer();

#endif // PARSING_MNIST_H
