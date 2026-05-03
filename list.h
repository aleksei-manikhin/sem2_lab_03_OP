#ifndef LIST_H
#define LIST_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Node {
    void* data;
    struct Node* next;
} Node;

typedef struct {
    size_t size;
    size_t dataSize;
    Node* head;
    Node* tail;
} List;

List* initList(size_t dataSize);
void disposeList(List* list);
void clearList(List* list);
int pushBack(List* list, const void* data);
int insertSorted(List* list, const void* data, int (*cmp)(const void*, const void*));


#ifdef __cplusplus
}
#endif

#endif // LIST_H
