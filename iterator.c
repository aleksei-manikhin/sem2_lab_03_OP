#include "iterator.h"

Iterator begin(const List* list) {
  Iterator it;
  it.current = NULL;

  if (list != NULL)
    it.current = list->head;

  return it;
}

int isSet(const Iterator* it) {
  return (it != NULL && it->current != NULL);
}

void next(Iterator* it) {
  if (it != NULL && it->current != NULL)
    it->current = it->current->next;
}

void* get(const Iterator* it) {
  return (it && it->current) ? it->current->data : NULL;
}

void* getNext(const Iterator* it) {
  return (it && it->current && it->current->next) ? it->current->next->data : NULL;
}
