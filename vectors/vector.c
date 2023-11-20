// /vectors/vector.c

#include "vector.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


Vector initVector(double coordinates[784], char* name) {
	if (!name) {
		perror("Provided name is NULL in initVector.\n");
		exit(EXIT_FAILURE);
	}
    Vector v;
	memset(v.name, 0, sizeof(v.name));  // Initialize the name buffer to zeros

    for (int i = 0; i < 784; i++) {
        v.coordinates[i] = coordinates[i];
    }
	strncpy(v.name, name, 255);  // Copy at most 255 characters
    v.name[255] = '\0'; // Ensure null termination
    return v;
}