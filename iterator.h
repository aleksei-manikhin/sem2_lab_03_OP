#ifndef ITERATOR_H
#define ITERATOR_H

#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Iterator {
    Node* current;
} Iterator;

Iterator begin(const List* list);
int isSet(const Iterator* it);
void next(Iterator* it);
void* get(const Iterator* it);
void* getNext(const Iterator* it);

#ifdef __cplusplus
}
#endif

#endif // ITERATOR_H
