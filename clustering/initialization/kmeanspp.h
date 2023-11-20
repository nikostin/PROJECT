#ifndef KMEANSPP_H
#define KMEANSPP_H

#include "../../lists/list.h"
#include "../../vectors/vector.h"
#include "../assignment/lloyd.h"

// Function to perform K-Means++ initialization
List* kmeansPlusPlusInitialization(List* data, int K);

#endif // KMEANSPP_H
