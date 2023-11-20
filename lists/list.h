// /lists/list.h

#ifndef LIST_H
#define LIST_H

#include "../vectors/vector.h"

typedef struct Node {
    Vector vector;
    struct Node* next;
} Node;

typedef struct {
    Node* head;
	Node* tail; 
    int size;
} List;

List* initializeList();
void insertToList(List* list, Vector vector);
Vector* getVectorAt(List* list, int index);
void freeList(List* list);
int lengthOfList(List* list);
void appendList(List* mainList, List* appendFrom);
Node* getNthNode(List* list, int n);


#endif // LIST_H