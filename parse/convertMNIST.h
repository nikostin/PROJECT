#ifndef CONVERT_MNIST_H
#define CONVERT_MNIST_H

#include <stdint.h>

void convertMNIST(const char *inputFileName);
int32_t readInt(FILE *fp);
void createQueryDatFromIDX(const char* dirPath);

#endif // CONVERT_MNIST_H
