/*
 * Copyright ©2025 Hal Perkins.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Winter Quarter 2025 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <stdio.h>
#include <stdlib.h>

#include "CSE333.h"
#include "LinkedList.h"
#include "LinkedList_priv.h"


///////////////////////////////////////////////////////////////////////////////
// LinkedList implementation.

LinkedList* LinkedList_Allocate(void) {
  // Allocate the linked list record.
  LinkedList *ll = (LinkedList *) malloc(sizeof(LinkedList));
  Verify333(ll != NULL);

  // STEP 1: initialize the newly allocated record structure.
  ll->num_elements = 0;
  ll->head = NULL;
  ll->tail = NULL;
  // Return our newly minted linked list.
  return ll;
}

void LinkedList_Free(LinkedList *list,
                     LLPayloadFreeFnPtr payload_free_function) {
  Verify333(list != NULL);
  Verify333(payload_free_function != NULL);

  // STEP 2: sweep through the list and free all of the nodes' payloads
  // (using the payload_free_function supplied as an argument) and
  // the nodes themselves.
  LinkedListNode *n = list->tail;
  for (int i = list->num_elements; i > 0; i--) {
    payload_free_function(n->payload);
    n = n->prev;
    list->num_elements--;
  }
  // free the LinkedList
  free(list);
}

int LinkedList_NumElements(LinkedList *list) {
  Verify333(list != NULL);
  return list->num_elements;
}

void LinkedList_Push(LinkedList *list, LLPayload_t payload) {
  Verify333(list != NULL);

  // Allocate space for the new node.
  LinkedListNode *ln = (LinkedListNode *) malloc(sizeof(LinkedListNode));
  Verify333(ln != NULL);

  // Set the payload
  ln->payload = payload;

  if (list->num_elements == 0) {
    // Degenerate case; list is currently empty
    Verify333(list->head == NULL);
    Verify333(list->tail == NULL);
    ln->next = ln->prev = NULL;
    list->head = list->tail = ln;
    list->num_elements = 1;
  } else {
    // STEP 3: typical case; list has >=1 elements
    Verify333(list->head->prev == NULL);
    ln->next = ln->prev = NULL;
    list->head->prev = ln;
    ln->next = list->head;
    list->head = ln;
    list->num_elements++;
  }
}

bool LinkedList_Pop(LinkedList *list, LLPayload_t *payload_ptr) {
  Verify333(payload_ptr != NULL);
  Verify333(list != NULL);

  // STEP 4: implement LinkedList_Pop.  Make sure you test for
  // and empty list and fail.  If the list is non-empty, there
  // are two cases to consider: (a) a list with a single element in it
  // and (b) the general case of a list with >=2 elements in it.
  // Be sure to call free() to deallocate the memory that was
  // previously allocated by LinkedList_Push().
  if (list->num_elements == 1) {
    // check that head and tail pointers are not null
    Verify333(list->head != NULL);
    Verify333(list->tail != NULL);
    // set pointer to payload at the head of LinkedList
    *payload_ptr = list->head->payload;
    // clear values in head and tail
    list->head = list->tail = NULL;
    // decrement list elements count
    list->num_elements = 0;
    // free memory allocated in push
    free(list->head);
    return true;
  } else if (list->num_elements > 1) {
    // check that head and tail pointers are not null
    Verify333(list->head != NULL);
    Verify333(list->tail != NULL);

    // set pointer to payload at the head of LinkedList
    *payload_ptr = list->head->payload;
    // set list head pointer to next node
    list->head = list->head->next;
    // remove prev head from list
    list->head->prev = list->tail->next = NULL;
    // decrement num_elements counter
    list->num_elements--;
    // free memory allocated by push
    free(list->head->prev);
    return true;
  } 
  // return false for empty lists
  return false;
}

void LinkedList_Append(LinkedList *list, LLPayload_t payload) {
  Verify333(list != NULL);

  // STEP 5: implement LinkedList_Append.  It's kind of like
  // LinkedList_Push, but obviously you need to add to the end
  // instead of the beginning.

  // Allocate space for the new node.
  LinkedListNode *ln = (LinkedListNode *) malloc(sizeof(LinkedListNode));
  Verify333(ln != NULL);

  // Set the payload
  ln->payload = payload;

  if (list->num_elements == 0) {
    // Degenerate case; list is currently empty
    Verify333(list->head == NULL);
    Verify333(list->tail == NULL);
    ln->next = ln->prev = NULL;
    list->head = list->tail = ln;
    list->num_elements = 1;
  } else {
    Verify333(list->tail->next == NULL);
    ln->next = ln->prev = NULL;
    list->tail->next = ln;
    ln->prev = list->tail;
    list->tail = ln;
    list->num_elements++;
  }
}

void LinkedList_Sort(LinkedList *list, bool ascending,
                     LLPayloadComparatorFnPtr comparator_function) {
  Verify333(list != NULL);
  if (list->num_elements < 2) {
    // No sorting needed.
    return;
  }

  // We'll implement bubblesort! Nnice and easy, and nice and slow :)
  int swapped;
  do {
    LinkedListNode *curnode;

    swapped = 0;
    curnode = list->head;
    while (curnode->next != NULL) {
      int compare_result = comparator_function(curnode->payload,
                                               curnode->next->payload);
      if (ascending) {
        compare_result *= -1;
      }
      if (compare_result < 0) {
        // Bubble-swap the payloads.
        LLPayload_t tmp;
        tmp = curnode->payload;
        curnode->payload = curnode->next->payload;
        curnode->next->payload = tmp;
        swapped = 1;
      }
      curnode = curnode->next;
    }
  } while (swapped);
}


///////////////////////////////////////////////////////////////////////////////
// LLIterator implementation.

LLIterator* LLIterator_Allocate(LinkedList *list) {
  Verify333(list != NULL);

  // OK, let's manufacture an iterator.
  LLIterator *li = (LLIterator *) malloc(sizeof(LLIterator));
  Verify333(li != NULL);

  // Set up the iterator.
  li->list = list;
  li->node = list->head;

  return li;
}

void LLIterator_Free(LLIterator *iter) {
  Verify333(iter != NULL);
  free(iter);
}

bool LLIterator_IsValid(LLIterator *iter) {
  Verify333(iter != NULL);
  Verify333(iter->list != NULL);

  return (iter->node != NULL);
}

bool LLIterator_Next(LLIterator *iter) {
  Verify333(iter != NULL);
  Verify333(iter->list != NULL);
  Verify333(iter->node != NULL);

  // STEP 6: try to advance iterator to the next node and return true if
  // you succeed, false otherwise
  // Note that if the iterator is already at the last node,
  // you should move the iterator past the end of the list
  // check that there is a next node
  if (iter->node->next != NULL) {
    // update current node and return true
    iter->node = iter->node->next;
    return true;
  }
  // otherwise set current node to null and return false
  iter->node = NULL;
  return false;  // you may need to change this return value
}

void LLIterator_Get(LLIterator *iter, LLPayload_t *payload) {
  Verify333(iter != NULL);
  Verify333(iter->list != NULL);
  Verify333(iter->node != NULL);

  *payload = iter->node->payload;
}

bool LLIterator_Remove(LLIterator *iter,
                       LLPayloadFreeFnPtr payload_free_function) {
  Verify333(iter != NULL);
  Verify333(iter->list != NULL);
  Verify333(iter->node != NULL);

  // STEP 7: implement LLIterator_Remove.  This is the most
  // complex function you'll build.  There are several cases
  // to consider:
  // - degenerate case: the list becomes empty after deleting.
  // - degenerate case: iter points at head
  // - degenerate case: iter points at tail
  // - fully general case: iter points in the middle of a list,
  //                       and you have to "splice".
  //
  // Be sure to call the payload_free_function to free the payload
  // the iterator is pointing to, and also free any LinkedList
  // data structure element as appropriate.

  // handle all four cases
  if (iter->list->num_elements == 1) {  // only one node in list
    payload_free_function(iter->node->payload);
    iter->list->num_elements--;
    iter->list->head = iter->list->tail = iter->node = NULL;
    iter = NULL;
    return false;
  } else if (iter->node == iter->list->head) {  // current node is also head of list
    payload_free_function(iter->node->payload);
    iter->list->head = iter->list->head->next;
    iter->list->num_elements--;
    iter->node = iter->node->next;
    iter->node->prev = NULL;
    return true;
  } else if (iter->node == iter->list->tail) {  // current node is also tail of list
    payload_free_function(iter->list->tail->payload);
    iter->node = iter->node->prev;
    return LLSlice(iter->list, &iter->list->tail->payload);
  } else {  // current node is somewhere in the middle of the list
    payload_free_function(iter->node->payload);
    iter->node->prev->next = iter->node->next;
    iter->node->next->prev = iter->node->prev;
    iter->node = iter->node->next;
    iter->list->num_elements--;
    return true;
  }
}


///////////////////////////////////////////////////////////////////////////////
// Helper functions

bool LLSlice(LinkedList *list, LLPayload_t *payload_ptr) {
  Verify333(payload_ptr != NULL);
  Verify333(list != NULL);

  // STEP 8: implement LLSlice.
  if (list->num_elements == 1) {
    // check that head and tail pointers are not null
    Verify333(list->head != NULL);
    Verify333(list->tail != NULL);
    // set pointer to payload at the head of LinkedList
    *payload_ptr = list->tail->payload;
    // empty list and return true
    list->head = list->tail = NULL;
    list->num_elements = 0;
    // free memory allocated 
    free(list->head);
    return true;
  } else if (list->num_elements > 1) {
    // check that head and tail pointers are not null
    Verify333(list->head != NULL);
    Verify333(list->tail != NULL);
    // set pointer to payloat at the head of LinkedList
    *payload_ptr = list->tail->payload;
    // set list tail pointer to prev node
    list->tail = list->tail->prev;

    // remove prev tail from list
    list->head->prev = list->tail->next = NULL;
    // decrement num_elements counter 
    list->num_elements--;
    // free memory allocated by append
    free(list->tail->next);
    return true;
  } 
  // return false for empty lists
  return false;
}

void LLIteratorRewind(LLIterator *iter) {
  iter->node = iter->list->head;
}
