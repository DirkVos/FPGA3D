#include "clist.h"

/*! \file clist.c
  \brief Doubly linked list.
  \author cxnf
  \version 1
  \date 2013-10-21
  \copyright GNU Public License
*/

#include <stddef.h>
#include <stdlib.h>


// ----------------- Local Variables --------------------------------------
static Iterator _NULL_iterator_ = { NULL, NULL, NULL, NULL }; //!< NULL iterator, allows functions to return a valid pointer to an invalid iterator without creating potential memory leaks.

// ----------------- Local Function declarations --------------------------

/*! \brief Get the 'NULL iterator'.
  Returns a valid pointer to an invalid iterator without requirement to allocate a new iterator.
  \return Invalid iterator.
*/
static inline Iterator * getNullIterator();

// ----------------- Global Function definitions --------------------------
List * createList(Deallocator dealloc) {
  List * list = (List *)malloc(sizeof(struct List));
  // do not try to initialize a list when memory was not allocated
  if (!list) {
    return NULL;
  }
  // set list to empty state
  list->first = NULL;
  list->last = NULL;
  list->uiSize = 0;
  list->dealloc = dealloc;
  return list;
}

void clearList(List * list) {
  // do not try to clear a non-existing list
  if (!list) {
    return;
  }
  Iterator * current = list->first;
  Iterator * next = NULL;
  // while list has another element, save pointer to next (may be invalid, but checked on next iteration) and free allocated memory (item and iterator)
  while (current) {
    next = current->next;
    if (list->dealloc) {
      (*list->dealloc)(current->pItem);
    }
    free(current);
    current = next;
  }
  // reset list to empty state
  list->first = NULL;
  list->last = NULL;
  list->uiSize = 0;
}

void destroyList(List * list) {
  // do not try to destroy when we got nothing to destroy
  if (!list) {
    return;
  }
  // clear frees all elements (item and iterator), only then list can be safely deallocated
  clearList(list);
  free(list);
}

Iterator * getBegin(List * list) {
  // return special 'invalid' iterator when no list was given
  if (!list) {
    return getNullIterator();
  }
  return list->first;
}
Iterator * getEnd(List * list) {
  // return special 'invalid' iterator when no list was given
  if (!list) {
    return getNullIterator();
  }
  return list->last;
}

uint32_t getSize(List * list) {
  // do not try to get size of non existing list
  if (!list) {
    return 0;
  }
  return list->uiSize;
}

Iterator * getEntry(List * list, uint32_t uiIndex) {
  // return special 'invalid' iterator when no list was given
  if (!list) {
    return getNullIterator();
  }
  uint32_t i;
  Iterator * iter;
  // when requested index is above half, iterate backwards to element at index, otherwise use foreward iteration
  if (list->uiSize / 2 < uiIndex) {
    for (i = list->uiSize - 1, iter = getEnd(list); i > uiIndex && iter; --i, movePrevious(&iter));
  } else {
    for (i = 0, iter = getBegin(list); i < uiIndex && iter; ++i, moveNext(&iter));
  }
  // make sure iterator is at current index AND iterator is within list bounds
  if (i != uiIndex || !iter) {
    return getNullIterator();
  } else {
    return iter;
  }
}

int8_t addEntry(List * list, Iterator * iterator, void * pItem) {
  // do not try to add to non existing list or add non existing item
  if (!list || ! pItem) {
    return 0;
  }
  // set iterator to 'invalid' iterator when non was given, this results in putting the item in front of all others
  if (!iterator) {
    iterator = getNullIterator();
  } else if (list != iterator->container) {
    return 0;
  }
  Iterator * newEntry = (Iterator *)malloc(sizeof(struct Iterator));
  // do not try to initialize and add entry when memory is not allocated
  if (!newEntry) {
    return 0;
  }
  newEntry->prev = NULL;
  newEntry->next = NULL;
  newEntry->container = list;
  newEntry->pItem = pItem;
  ++list->uiSize;
  // if iterator is last push element to end of list, do not link new iterator to current as current might be special 'invalid' iterator
  // otherwise push it before iterator following current, do not link new iterator to current as current might be special 'invalid' iterator
  if (!iterator->next) {
    list->last = newEntry;
  } else {
    newEntry->next = iterator->next;
    newEntry->next->prev = newEntry;
  }
  // if iterator is special 'invalid' one, push to begining, do link new iterator to first iterator (if any)
  // otherwise push it directly behind current iterator
  if (!iterator->pItem) {
    newEntry->next = list->first;
    if (newEntry->next) {
      newEntry->next->prev = newEntry;
    }
    list->first = newEntry;
  } else {
    newEntry->prev = iterator;
    iterator->next = newEntry;
  }
  return 1;
}

int8_t removeEntry(List * list, Iterator * iterator) {
  if (!list || !iterator) {
    return 0;
  }
  if (list != iterator->container || !iterator->pItem) {
    return 0;
  }
  --list->uiSize;
  if (!iterator->prev) {
    list->first = iterator->next;
  } else {
    iterator->prev->next = iterator->next;
  }
  if (!iterator->next) {
    list->last = iterator->prev;
  } else {
    iterator->next->prev = iterator->prev;
  }
  free(iterator->pItem);
  free(iterator);
  return 1;
}

void * getCurrent(Iterator * iterator) {
  if (!iterator) {
    return NULL;
  }
  return iterator->pItem;
}

int8_t moveNext(Iterator ** pIterator) {
  if (!pIterator) {
    return 0;
  }
  Iterator * iterator = *pIterator;
  if (!iterator) {
    return 0;
  }
  *pIterator = iterator->next;
  return 1;
}
int8_t movePrevious(Iterator ** pIterator) {
  if (!pIterator) {
    return 0;
  }
  Iterator * iterator = *pIterator;
  if (!iterator) {
    return 0;
  }
  *pIterator = iterator->prev;
  return 1;
}

int8_t isBegin(Iterator * iterator) {
  if (!iterator) {
    return 1;
  }
  if (!iterator->pItem) {
    return 1;
  }
  return !iterator->prev || iterator->container->first == iterator;
}
int8_t isEnd(Iterator * iterator) {
  if (!iterator) {
    return 1;
  }
  if (!iterator->pItem) {
    return 1;
  }
  return !iterator->next || iterator->container->last == iterator;
}


// ----------------- Local Function definitions ---------------------------
static inline Iterator * getNullIterator() {
  return &_NULL_iterator_;
}
