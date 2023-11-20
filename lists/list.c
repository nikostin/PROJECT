#include <stdlib.h>
#include "../vectors/vector.h"
#include "list.h"
#include <stdio.h>  // Added for debug prints

List* initializeList() {
    List* newList = (List*)malloc(sizeof(List));
    if (!newList) {  // Add this check
        perror("Failed to allocate memory for new list");
        exit(EXIT_FAILURE);
    }
    newList->head = NULL;
    newList->tail = NULL;  
    newList->size = 0;
    return newList;
}

void insertToList(List* list, Vector vector) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->vector = vector;
    newNode->next = NULL;

    if (!list->head) {
        list->head = newNode;
        list->tail = newNode;  
    } else {
        list->tail->next = newNode;
        list->tail = newNode;  
    }
    list->size++;

    // Debug print
    //printf(" Vector inserted to list. New size: %d", list->size);
}

Vector* getVectorAt(List* list, int index) {
    if (index < 0 || index >= list->size) {
        return NULL;
    }

    Node* current = list->head;
    for (int i = 0; i < index; i++) {
        current = current->next;
    }
    return &(current->vector);
}

void freeList(List* list) {
    // Check if the list nodes are in an array (the next pointer of the tail is NULL)
    if (list->tail && list->tail->next == NULL) {
        // If the nodes are in an array, free the entire array at once
        free(list->head);
    } else {
        // If the nodes are individual allocations, free them one by one
        Node* current = list->head;
        while (current) {
            Node* nextNode = current->next;
            free(current);
            current = nextNode;
        }
    }
    // Reset the head and tail pointers of the list to NULL
    list->head = NULL;
    list->tail = NULL;
    
    // Finally, free the list structure itself
    free(list);
}


int lengthOfList(List* list) {
    return list->size; // Updated to return size attribute
}

void appendList(List* mainList, List* appendFrom) {
    if (mainList == NULL || appendFrom == NULL) {
        return;
    }
    if (mainList->head == NULL) {
        mainList->head = appendFrom->head;
        mainList->tail = appendFrom->tail;
    } else {
        mainList->tail->next = appendFrom->head;
        mainList->tail = appendFrom->tail;
    }
    mainList->size += appendFrom->size;  // Update size
    appendFrom->head = NULL;
    appendFrom->tail = NULL;
    appendFrom->size = 0;  // Reset size of appendFrom
}

Node* getNthNode(List* list, int n) {
    Node* node = list->head;
    int count = 1; // start count at 1
    while (node) {
        if (count == n) {
            return node;
        }
        node = node->next;
        count++;
    }
    return NULL; // return NULL if n is greater than the length of the list
}
