// /vectors/vector.h

#ifndef VECTOR_H
#define VECTOR_H

typedef struct {
    double coordinates[784]; // Since each image transforms into a 784-dimensional vector
    char name[256]; // Name/ID for the vector, e.g., Image123
} Vector;

Vector initVector(double coordinates[784], char* name);


#endif // VECTOR_H
