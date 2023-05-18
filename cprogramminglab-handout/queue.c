/* 
 * Code for basic C skills diagnostic.
 * Developed for courses 15-213/18-213/15-513 by R. E. Bryant, 2017
 * Modified to store strings, 2018
 */

/*
 * This program implements a queue supporting both FIFO and LIFO
 * operations.
 *
 * It uses a singly-linked list to represent the set of queue elements
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "harness.h"
#include "queue.h"


list_ele_t *node_alloc(char *s) {
  list_ele_t *ptr = malloc(sizeof(list_ele_t));
  if (ptr == NULL) return NULL;

  ptr->next = NULL;
  ptr->value = malloc(strlen(s) + 1);
  if (ptr->value == NULL) {
    free(ptr);
    return NULL;
  }
  strcpy(ptr->value, s);

  return ptr;
}

void node_free(list_ele_t *ptr) {
  if (ptr == NULL) return;
  free(ptr->value);
  free(ptr);
}

/*
  Create empty queue.
  Return NULL if could not allocate space.
*/
queue_t *q_new()
{
    queue_t *q =  malloc(sizeof(queue_t));
    if (q == NULL) return NULL;
    q->head = NULL;
    q->tail = NULL;
    q->len = 0;
    return q;
}

/* Free all storage used by queue */
void q_free(queue_t *q)
{
    /* How about freeing the list elements and the strings? */
    /* Free queue structure */
    if (q == NULL) return;
    for (int i = 0; i < q->len; i++) {
      list_ele_t *ptr = q->head;
      q->head = ptr->next;
      node_free(ptr);
    }
    free(q);
}

/*
  Attempt to insert element at head of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(queue_t *q, char *s)
{
    if (q == NULL) return false;
    list_ele_t *newh = node_alloc(s);
    if (newh == NULL) return false;

    newh->next = q->head;
    q->head = newh->next;
    if (q->len++ == 1) q->tail = newh;

    return true;
}


/*
  Attempt to insert element at tail of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(queue_t *q, char *s)
{
    /* You need to write the complete code for this function */
    /* Remember: It should operate in O(1) time */
    if (q == NULL) return false;
    list_ele_t *newt = node_alloc(s);
    if (newt == NULL) return false;

    q->len++;
    if (q->len == 1)
      q->tail = q->head = newt;
    else {
      q->tail->next = newt;
      q->tail = newt;
    }
    return true;
}

/*
  Attempt to remove element from head of queue.
  Return true if successful.
  Return false if queue is NULL or empty.
  If sp is non-NULL and an element is removed, copy the removed string to *sp
  (up to a maximum of bufsize-1 characters, plus a null terminator.)
  The space used by the list element and the string should be freed.
*/
bool q_remove_head(queue_t *q, char *sp, size_t bufsize)
{
    /* You need to fix up this code. */
    if (q == NULL || q->len == 0) return false;
    list_ele_t *oldhead = q->head;
    if (sp != NULL)
      strncpy(sp, oldhead->value, bufsize - 1);
    q->head = q->head->next;
    q->len--;
    node_free(oldhead);
    if (q->len == 0) q->tail = NULL;
    return true;
}

/*
  Return number of elements in queue.
  Return 0 if q is NULL or empty
 */
int q_size(queue_t *q)
{
    /* You need to write the code for this function */
    /* Remember: It should operate in O(1) time */
    if (q == NULL) return 0;
    return q->len;
}

/*
  Reverse elements in queue
  No effect if q is NULL or empty
  This function should not allocate or free any list elements
  (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
  It should rearrange the existing ones.
 */
void q_reverse(queue_t *q)
{
    /* You need to write the code for this function */
    if (q == NULL || q->len <= 1) return;
    list_ele_t *newh = NULL;
    q->tail = q->head;
    for (int i = 0; i < q->len; i++) {
      list_ele_t *temp = q->head->next;
      q->head->next = newh;
      newh = q->head;
      q->head = temp;
    }
    q->head = newh;
}

