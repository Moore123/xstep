/* double linked list.c - A generic doubly linked list implementation
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <ck_pr.h>
#include <ck_rwlock.h>

#include "list.h"
#include "pthreadpool.h"

// #include "zmalloc.h"

/* Create a new list. The created list can be freed with
 * AlFreeList(), but private value of every node need to be freed
 * by the user before to call AlFreeList().
 *
 * On error, NULL is returned. Otherwise the pointer to the new list. */
extern int ndebug(char *, ...);
list *ListCreate(void)
{
	list *list;

	if ((list = malloc(sizeof(*list))) == NULL)
		return NULL;

 	ck_rwlock_init(&list->entity_lock);

    pthread_mutexattr_t mutexattr;

    pthread_mutexattr_init(&mutexattr);
    //pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutexattr_setkind_np (&mutexattr, PTHREAD_MUTEX_ERRORCHECK_NP);

	//list->mutex_lock = calloc(1, sizeof(pthread_mutex_t));
	if (pthread_mutex_init(&list->mutex_lock, NULL) != 0) {
        pthread_mutexattr_destroy(&mutexattr);
		free(list);
		return NULL;
	}
   pthread_cond_init(&list->cwait,NULL);

   pthread_mutexattr_destroy(&mutexattr);

#ifdef _PTHREAD_H
	list->sort_threads = 4;
#endif

	list->head = list->tail = NULL;
	list->len =  0;
    list->vsize = NULL;
	list->dup = NULL;
	list->free = NULL;
	list->match = NULL;

	return list;
}

/* Free the whole list.
 *
 * This function can't fail. */
void ListRelease(list * list)
{

	listNode *current, *next;

    if ( list == NULL ) return;

	if ( (current = list->head ) == NULL ) return;

	while (current) {
		next = current->next;
		if (list->free)
			list->free(current->value);
		free(current);
		current = next;
	}

	pthread_mutex_destroy(&list->mutex_lock);

	free(list);
}

/* Add a new node to the list, to head, contaning the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
list *ListAddNodeHead(list *list,  void *value)
{
	listNode *node;

    do {

    if ( list == NULL ) break;
	if ((node = malloc(sizeof(*node))) == NULL) break;

	node->value = value;

	if (list->len == 0) {
		list->head = node;
		list->tail = node;
		node->prev = node->next = NULL;
	} else {
		node->prev = NULL;
		node->next = list->head;
		list->head->prev = node;
		list->head = node;
	}
	(list)->len+=1;
    } while(0);

	return list;
}

/* Add a new node to the list, to tail, containing the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
list *ListAddNodeTail(list * list,  void *value)
{
	listNode *node;

    do {

    if ( list == NULL ) break;

	if ((node =(listNode *)calloc(1,sizeof(*node))) == NULL) break;
	
	node->value = value;

	if (list->len == 0) {
		node->prev = node->next = NULL;
		list->head = node;
		list->tail = node;
	} else {
		node->prev = list->tail;
		node->next = NULL;
		list->tail->next = node;
		list->tail=node;
	}

	list->len+=1;

    } while(0);

	return list;
}

int list_empty(list * ll)
{
	return (listLength(ll));
}

list *listInsertNode(list * list, listNode * old_node,  void *value, int after)
{
	listNode *node;

    if ( list == NULL ) return(NULL);

	if ((node = malloc(sizeof(*node))) == NULL)
		return NULL;
	node->value = value;
	if (after) {
		node->prev = old_node;
		node->next = old_node->next;
		if (list->tail == old_node) {
			list->tail = node;
		}
	} else {
		node->next = old_node;
		node->prev = old_node->prev;
		if (list->head == old_node) {
			list->head = node;
		}
	}
	if (node->prev != NULL) {
		node->prev->next = node;
	}
	if (node->next != NULL) {
		node->next->prev = node;
	}
	list->len++;
	return list;
}

/* Remove the specified node from the specified list.
 * It's up to the caller to free the private value of the node.
 *
 * This function can't fail. */
void listDelNode(list * list, listNode * node)
{
    if ( list == NULL ) return;
    if ( node == NULL ) return;

	if (node->prev) 
		node->prev->next = node->next;
	else
		list->head = node->next;

	if (node->next) 
		node->next->prev = node->prev;
	else
		list->tail = node->prev;
        
	if ( (list->free) && ( node->value ) )
		list->free(node->value);
	free(node);
	list->len--;

    return;
}

/* Returns a list iterator 'iter'. After the initialization every
 * call to ListNext() will return the next element of the list.
 *
 * This function can't fail. */
listIter *listGetIterator(list * list, int direction)
{
	listIter *iter;
    if ( list == NULL ) return(NULL);
	if ((iter = malloc(sizeof(*iter))) == NULL)
		return NULL;
	if (direction == AL_START_HEAD)
		iter->next = list->head;
	else
		iter->next = list->tail;
	iter->direction = direction;
	return iter;
}

/* Release the iterator memory */
void ListReleaseIterator(listIter * iter)
{
	free(iter);
}

/* Create an iterator in the list private iterator structure */
void listRewind(list * list, listIter * li)
{
    if ( list == NULL ) return;

	li->next = list->head;
	li->direction = AL_START_HEAD;
}

void listRewindTail(list * list, listIter * li)
{
    if ( list == NULL ) return;

	li->next = list->tail;
	li->direction = AL_START_TAIL;
}

/* Return the next element of an iterator.
 * It's valid to remove the currently returned element using
 * listDelNode(), but not to remove other elements.
 *
 * The function returns a pointer to the next element of the list,
 * or NULL if there are no more elements, so the classical usage patter
 * is:
 *
 * iter = listGetIterator(list,<direction>);
 * while ((node = ListNext(iter)) != NULL) {
 *     doSomethingWith(listNodeValue(node));
 * }
 *
 * */
listNode *ListNext(listIter * iter)
{
	listNode *current = iter->next;

	if (current != NULL) {
		if (iter->direction == AL_START_HEAD)
			iter->next = current->next;
		else
			iter->next = current->prev;
	}
	return current;
}

/* Duplicate the whole list. On out of memory NULL is returned.
 * On success a copy of the original list is returned.
 *
 * The 'Dup' method set with listSetDupMethod() function is used
 * to copy the node value. Otherwise the same pointer value of
 * the original node is used as value of the copied node.
 *
 * The original list both on success or error is never modified. */
list *ListDup(list * orig)
{
	list *copy;
	listIter *iter;
	listNode *node;

    if ( orig == NULL ) return(NULL);

	if ((copy = ListCreate()) == NULL)
		return NULL;
	copy->dup = orig->dup;
	copy->free = orig->free;
	copy->match = orig->match;

    ListEachFromHead(orig,iter,node) {
		void *value;

		if (copy->dup) {
			value = copy->dup(node->value);
			if (value == NULL) {
				ListRelease(copy);
				ListReleaseIterator(iter);
				return NULL;
			}
		} else
			value = node->value;
		if (ListAddNodeTail(copy, value) == NULL) {
			ListRelease(copy);
			ListReleaseIterator(iter);
			return NULL;
		}
	} ListEachEnd(iter);

	return copy;
}

/* Search the list for a node matching a given key.
 * The match is performed using the 'match' method
 * set with listSetMatchMethod(). If no 'match' method
 * is set, the 'value' pointer of every node is directly
 * compared with the 'key' pointer.
 *
 * On success the first matching node pointer is returned
 * (search starts from head). If no matching node exists
 * NULL is returned. */
listNode *listSearchKey(list * list, void *key)
{
	listIter *iter;
	listNode *node;

    if ( list == NULL ) return(NULL);

    ListEachFromHead(list,iter,node) {
		if (list->match) {
			if (list->match(node->value, key)) {
				ListReleaseIterator(iter);
				return node;
			}
		} else {
			if (key == node->value) {
				ListReleaseIterator(iter);
				return node;
			}
		}
	} ListEachEnd(iter);

	return NULL;
}

/* Return the element at the specified zero-based index
 * where 0 is the head, 1 is the element next to head
 * and so on. Negative integers are used in order to count
 * from the tail, -1 is the last element, -2 the penultimate
 * and so on. If the index is out of range NULL is returned. */
listNode *listIndex(list * list, long index)
{
	listNode *n;

    if ( list == NULL ) return(NULL);

	if (index < 0) {
		index = (-index) - 1;
		n = list->tail;
		while (index-- && n)
			n = n->prev;
	} else {
		n = list->head;
		while (index-- && n)
			n = n->next;
	}
	return n;
}

/* Rotate the list removing the tail node and inserting it to the head. */
void listRotate(list * list)
{
	listNode *tail = list->tail;

    if ( list == NULL ) return;

	if (listLength(list) <= 1)
		return;

	/* Detach current tail */
	list->tail = tail->prev;
	list->tail->next = NULL;
	/* Move it as head */
	list->head->prev = tail;
	tail->prev = NULL;
	tail->next = list->head;
	list->head = tail;
}

void _swap_nvalues(listNode * a, listNode * b)
{
	void *temp_value = a->value;
	a->value = b->value;
	b->value = temp_value;
}

void ListSort(list * ll_which, int (*cmp) (const void *, const void *))
{

	listNode *first, *target;

    if ( ll_which == NULL ) return;

	first = ll_which->head;

	while (first) {

		target = first->next;
		while (target != NULL) {
			if (cmp(first->value, target->value) == 1) {
				_swap_nvalues(first, target);
			}
			target = target->next;
		}
		first = first->next;
	}

	return;

}

/* Quick Sort List this version is definitely slowly */

void listQuickSort(listNode * pLeft, listNode * pRight, int (*cmp) (const void *, const void *))
{
	listNode *pStart;
	listNode *pCurrent;
	void *nCopyValue;

	// If the left and right pointers are the same, then return
	if (pLeft == pRight)
		return;

	// Set the Start and the Current item pointers
	pStart = pLeft;
	pCurrent = pStart->next;

	// Loop forever (well until we get to the right)
	while (1) {
		// If the start item is less then the right
		if (cmp(pStart->value, pCurrent->value) == TRUE) {
			// Swap the items
			nCopyValue = pCurrent->value;
			pCurrent->value = pStart->value;
			pStart->value = nCopyValue;
		}
		// Check if we have reached the right end
		if (pCurrent == pRight)
			break;

		// Move to the next item in the list
		pCurrent = pCurrent->next;
	}

	// Swap the First and Current items
	nCopyValue = pLeft->value;
	pLeft->value = pCurrent->value;
	pCurrent->value = nCopyValue;

	// Save this Current item
	listNode *pOldCurrent = pCurrent;

	// Check if we need to sort the left hand size of the Current point
	pCurrent = pCurrent->prev;
	if (pCurrent != NULL) {
		if ((pLeft->prev != pCurrent) && (pCurrent->next != pLeft))
			listQuickSort(pLeft, pCurrent, cmp);
	}
	// Check if we need to sort the right hand size of the Current point
	pCurrent = pOldCurrent;
	pCurrent = pCurrent->next;
	if (pCurrent != NULL) {
		if ((pCurrent->prev != pRight) && (pRight->next != pCurrent))
			listQuickSort(pCurrent, pRight, cmp);
	}
}

listNode **ListMap2VectorAddr(list *ll_which) {
    int i=0;
	listNode **loc;
	listNode *node;
    listIter *iter;

    unsigned int align = listLength(ll_which);

	if ( ( loc = calloc(align,sizeof(listNode *) ) ) == NULL ) return(NULL);
    ListEachFromHead(ll_which,iter,node) {
        loc[i++] = node;
    } ListEachEnd(iter);
   return(loc);
}

void ListMerge(list *dst_list, list *src_list)
{

	if( (src_list == NULL) || ( dst_list == NULL ) ) return;
    
	do {

    if ( listLength(src_list) == 0 ) break;

    if ( listLength(dst_list) == 0 ) {
        list_trywrlock(dst_list);
        dst_list->head = src_list->head;
        dst_list->tail = src_list->tail;
        dst_list->len = listLength(src_list);
        list_write_unlock(dst_list);
		free(src_list);
        break;
    }

        list_trywrlock(dst_list);
		dst_list->tail->next = src_list->head;
		src_list->head->prev = dst_list->tail;
		dst_list->tail = src_list->tail;
		dst_list->len += listLength(src_list);
        list_write_unlock(dst_list);

		pthread_mutex_destroy(&src_list->mutex_lock);
		free(src_list);

		return;
	} while (0);

}

void ListUnique(list *ll, int (*equ) (const void *, const void *),
				int (*merge)(const void *,const void *) ) {

	listNode  **nodes;
	listNode *current, *target=NULL, *chains=NULL;
	unsigned int lens=0, i , j;
	void *a, *b;

	do {

	if ( ( ll == NULL ) || (equ == NULL ) ) break;
	if ( (lens = listLength(ll) ) < 2 ) break;
 	while( (pthread_mutex_trylock(&ll->mutex_lock))!=0 ) ;;

	nodes = ListMap2VectorAddr(ll) ;

	for ( i = 0; i < listLength(ll)-1 ; i++ ) {

		if ( (listNode *)nodes[i] == NULL )  continue; 
		a = listNodeValue(nodes[i]);
		if ( a == NULL ) continue;

		for ( j = i+1; j < listLength(ll) ; j++ ) {

			if ( (listNode *)nodes[j] == NULL ) continue;
			b = listNodeValue(nodes[j]);
			if ( b == NULL ) continue;

			if ( equ(a,b) == TRUE ) {
				  if ( merge!= NULL ) merge(a,b); 
				  ll->free(nodes[j]->value);
				  nodes[j]->value = NULL; 
				  lens--;
			  }

	    }
	}
	
	lens = 0;
	for ( i = 0,j = 0; i < listLength(ll) ; i++ ) {

		if ( ( target = nodes[i] ) == NULL ) continue;
		if ( target->value == NULL ) { free(target) ; continue;  }
		if (i == 0) { chains = target; lens = 1; chains->prev = NULL; } 
		else {
		  current = nodes[j];
		  target->prev = current;
		  current->next = target;
		  target->next = NULL;
		  lens++;
	     }

		j = i;
 	}

	 ll->tail = nodes[j];
	 ll->head = chains;
	 ll->len = lens;

 	pthread_mutex_unlock(&ll->mutex_lock);

	free(nodes);

	} while(0);

	return;
}

void _qswap_nvalues(listNode ** loc, int a, int b)
{
	void *tmp = (void *)loc[a]->value;
	loc[a]->value = loc[b]->value;
	loc[b]->value = (void *) tmp;

	return;
}

void _qswap_addr(listNode ** loc, int a, int b)
{

	listNode *tmp = (listNode *) loc[a];
	loc[a] = loc[b];
	loc[b] = (listNode *) tmp;

	return;
}



void _ListQsort(listNode ** loc, int a, int b, int (*cmp) (const void *, const void *))
{

	listNode *la, *lb;
	listNode *tmp;
	int i, x, last;


//=======================================================
#if 0
	if ( (b-a) < 9) {
		for (i = b; i > a;  i--) 
		for (x = a; x < i; x++) {
		 if (cmp( ((listNode *)loc[x])->value, ((listNode *)loc[x+1])->value) == TRUE )  {
		 tmp = (listNode *)loc[x];
		 loc[x] = loc[x+1];
		 loc[x+1] = tmp;  
        }
			    
        }

		return;
	}
#endif
//=======================================================
	do {

	if ( loc == NULL ) break;
	if ( cmp == NULL ) break;

	if (a >= b) break;
	x = (a + b) / 2;

	if ( ( tmp = (listNode *)loc[a] ) == NULL ) break;

	loc[a] = loc[x];
	loc[x] = tmp;

	last = a;
	la = loc[a];

	if ( la == NULL ) break;

	for (i = a + 1; i <= b; i++) {
		if ( ( lb = loc[i] ) == NULL ) continue;

		if( ( la->value == NULL ) || ( lb->value == NULL ) )continue;

		if( cmp(la->value, lb->value) == TRUE ){
		   
			last++;
			if ( last > b ) break;
			if ( (tmp = (listNode *)loc[last]) == NULL ) break;
			loc[last] = loc[i];
			loc[i] = tmp; 
		}

	}

	tmp = (listNode *)loc[a];
	loc[a] = loc[last];
	loc[last] = tmp;

	_ListQsort(loc, a, last - 1, cmp);
	_ListQsort(loc, last + 1, b, cmp);

    } while(0);

	return;

}

void ListQsort(list *ll_which, int (*cmp) (const void *, const void *))
{
	int i = 0, len;
	listNode **loc;
	listNode *first, *target, *chains;

    if ( ll_which == NULL ) return;

	len = listLength(ll_which)-1;

   while(len > 0) {
 	while(pthread_mutex_trylock(&ll_which->mutex_lock)!=0);;

	if ( (loc = ListMap2VectorAddr(ll_which)) == NULL ) break;

	_ListQsort(loc, 0, len, cmp);

	//if swap_nvalues no need regenerate list
	//if qswap_addr uncomment the flow #if

	chains = loc[0];
	first = NULL;
	ll_which->head = chains;

	for (i = 0; i <(int) listLength(ll_which) ; i++) {
		if ( ( target = loc[i] ) == NULL ) continue;

		if (i == 0) {
		  chains = target;
		  chains->prev = NULL;

		} else {

		  first = loc[i-1];
		  target->prev = first;
		  first->next = target;
		  target->next = NULL;
		}
	}

	ll_which->tail = loc[i-1];
 	pthread_mutex_unlock(&ll_which->mutex_lock);

	free(loc);

	break;
   }
	return;
}

#ifdef _PTHREAD_H

void _serial_qsort(listNode **loc, int left, int right, int (*cmp) (const void *, const void *))
{
	int i, last,pivot;
	void *tmp;

	listNode *la, *lb;

	if (right > left) {

	pivot = (left + right) / 2;

	//========================================================
	//value SWAP(loc[left], loc[pivot]);
	tmp = (void *)loc[left]->value;
	loc[left]->value = loc[pivot]->value;
	loc[pivot]->value = tmp;

	last = left;
	la = loc[left];

	for (i = left + 1; i <= right; i++) {
		lb = loc[i];

		if (cmp(la->value, lb->value) == TRUE) {
			//value SWAP(loc[i], loc[last]);
			last++;
			tmp = (void *)loc[last]->value;
			loc[last]->value = loc[i]->value;
			loc[i]->value = tmp;
		}

	}

	//value SWAP(loc[left], loc[last]);
	tmp = (void *)loc[left]->value;
	loc[left]->value = loc[last]->value;
	loc[last]->value = tmp;

	//========================================================
	_serial_qsort(loc, left, last - 1, cmp);
	_serial_qsort(loc, last + 1, right, cmp);
	}
	return;
}

/**
  * Structure containing the arguments to the _list_p_qsort function.  Used
  * when starting it in a new thread, because pthread_create() can only pass one
  * (pointer) argument.
*/

struct psort_args {
	listNode **array;
	int left;
	int right;
	int depth;
	int (*cmp) (const void *, const void *);
};

void _list_p_qsort(listNode ** loc, int left, int right, int depth, int (*cmp) (const void *, const void *));

void *list_qs_thread(void *init)
{
	struct psort_args *start = init;
	_list_p_qsort(start->array, start->left, start->right, start->depth, start->cmp);
	return NULL;
}

void _list_p_qsort(listNode ** loc, int left, int right, int depth, int (*cmp) (const void *, const void *))
{
	int ret, pivot, i, last;
	void *tmp;

	listNode *la, *lb;

	if (right > left) {

	pivot = (right + left) / 2;

	//===========================================================
	//SWAP(loc[storeIndex], loc[right]);
	tmp = (void *)loc[left]->value;
	loc[left]->value = loc[pivot]->value;
	loc[pivot]->value = tmp;

	last = left;
	la = loc[left];

	for (i = left + 1; i <= right; i++) {
		lb = loc[i];

		if (cmp(la->value, lb->value) == TRUE) {
			last++;
			tmp = (void *)loc[last]->value;
			loc[last]->value = loc[i]->value;
			loc[i]->value = tmp;
		}

	}

	//SWAP(loc[storeIndex], loc[right]);
	tmp = (void *)loc[left]->value;
	loc[left]->value = loc[last]->value;
	loc[last]->value = tmp;

	//===========================================================
	// Either do the parallel or serial quicksort, depending on the depth
	// specified.

	if (depth-- > 0) {
		// Create the thread for the first recursive call
		struct psort_args arg = { loc, left, last - 1, depth, cmp };
		pthread_t thread;
		ret = pthread_create(&thread, NULL, list_qs_thread, &arg);
		assert((ret == 0) && "Thread creation failed");
		// Perform the second recursive call in this thread
		_list_p_qsort(loc, last + 1, right, depth, cmp);
		// Wait for the first call to finish.
		pthread_join(thread, NULL);
	} else {
		_serial_qsort(loc, left, last - 1, cmp);
		_serial_qsort(loc, last + 1, right, cmp);
	}
    }
   return;
}

void list_psort(list * ll_which, int (*cmp) (const void *, const void *))
{
	int len;
	listNode **loc;

    if ( ll_which == NULL ) return;

	len = listLength(ll_which) - 1;

	while( len > 0 ) {

	if ( (loc = ListMap2VectorAddr(ll_which)) == NULL ) break;

	_list_p_qsort(loc, 0, len, ll_which->sort_threads, cmp);

	free(loc);

	break;
	}

	return;
}

#endif

void _list_binsert(list * ll_which, listNode ** loc, listNode * xNode, int a, int b,
						 int (*cmp) (const void *, const void *))
{

	listNode *la, *lb;
	int x;

    if ( ll_which == NULL ) return;

	if (a > b)
		return;

	la = loc[a];
	lb = loc[b];

	if (a == b) {

		if (cmp(la->value, xNode->value) == TRUE) {
			xNode->prev = la->prev;
			if (la->prev != NULL)
				la->prev->next = xNode;
			xNode->next = la;
			la->prev = xNode;
			return;
		}

		if (cmp(xNode->value, lb->value) == TRUE) {
			xNode->next = lb->next;
			xNode->prev = lb;
			if (lb->next != NULL)
				lb->next->prev = xNode;
			lb->next = xNode;
			return;
		}
	}

	if ((a + 1) == b) {
		la = loc[a];
		lb = loc[b];
		if (cmp(la->value, xNode->value) == TRUE) {
			xNode->prev = la->prev;
			xNode->next = la;
			if (la->prev != NULL)
				la->prev->next = xNode;
			la->prev = xNode;
			return;
		}
		if (cmp(xNode->value, lb->value) == TRUE) {
			xNode->next = lb->next;
			xNode->prev = lb;
			if (lb->next != NULL)
				lb->next->prev = xNode;
			lb->next = xNode;
			return;
		}

		xNode->next = lb;
		xNode->prev = la;
		la->next = xNode;
		lb->prev = xNode;
		return;
	}

	x = (a + b) / 2;

	do {

		lb = loc[x];

		if (cmp(xNode->value, lb->value) != TRUE) {
			_list_binsert(ll_which, loc, xNode, a, x, cmp);
		} else {
			_list_binsert(ll_which, loc, xNode, x, b, cmp);
		}

	} while (0);

	return;

}

void list_binsert(list * ll_which, void *value, int (*cmp) (const void *, const void *))
{
	int i = 0, len;
	listNode **loc;
	listNode *first, *node;

    if ( ll_which == NULL ) return;

	len = listLength(ll_which);

	do {

		if (len == 0) {
			ListAddNodeHead(ll_which, value);
			break;
		}

		if (len == 1) {

			if (cmp(ll_which->head->value, value) == FALSE)
				ListAddNodeTail(ll_which, value);
			else
				ListAddNodeHead(ll_which, value);
			break;
		}

		if ((node = malloc(sizeof(*node))) == NULL) return;
		node->value = value;

		if ( (loc = ListMap2VectorAddr(ll_which)) == NULL ) break;

		_list_binsert(ll_which, loc, node, 0, len-1 , cmp);

		first = ll_which->head;
		while (first->prev) {
			first = first->prev;
		}
		ll_which->head = first;

		first = loc[i - 1];
		while (first->next) {
			first = first->next;
		}
		ll_which->tail = first;

		ll_which->len++;

		free(loc);

	} while (0);

	return;
}

void _left_seek(list * ll_which, listNode ** loc, void *key, list * target, int a, int b,
					 int (*cmp) (const void *, const void *))
{

	int x;
	listNode *la, *lb, *mid;

	if (a >= b)
		return;
	do {
		x = (a + b) / 2;
		mid = loc[x];
		la = loc[a];
		lb = loc[b];

		if (ll_which->match(la->value, key) == TRUE) {
			target->head = la;
			break;
		}

		if ((a + 1) == b) {
			target->head = lb;
			target->len = b;
			break;
		}

		if (ll_which->match(mid->value, key)) {
			_left_seek(ll_which, loc, key, target, a, x, cmp);
			break;
		}

		if (cmp(key, mid->value) == FALSE)
			_left_seek(ll_which, loc, key, target, a, x, cmp);
		else
			_left_seek(ll_which, loc, key, target, x, b, cmp);

	} while (0);

	return;

}

void _right_seek(list * ll_which, listNode ** loc, void *key, list * target, int a, int b,
					  int (*cmp) (const void *, const void *))
{
	int x;
	listNode *la, *lb, *mid;

	if (a >= b)
		return;
	do {
		x = (a + b) / 2;
		mid = loc[x];
		la = loc[a];
		lb = loc[b];

		if (ll_which->match(lb->value, key) == TRUE) {
			target->tail = lb;
			break;
		}

		if ((a + 1) == b) {
			target->tail = la;
			target->len = b - target->len - 1;
			break;
		}

		if (ll_which->match(mid->value, key)) {
			_right_seek(ll_which, loc, key, target, x, b, cmp);
			break;
		}

		if (cmp(key, mid->value) == FALSE)
			_right_seek(ll_which, loc, key, target, a, x, cmp);
		else
			_right_seek(ll_which, loc, key, target, x, b, cmp);

	} while (0);

	return;

}

void _list_dupbykey(list * ll_which, listNode ** loc, void *key, list * target, int a, int b,
						  int (*cmp) (const void *, const void *))
{
	int x;
	listNode *la, *lb, *mid;

	do {
		if (!(ll_which->match))
			break;
		if (a >= b)
			break;
		if ((a + 1) == b)
			break;

		x = (a + b) / 2;
		mid = loc[x];
		la = loc[a];
		lb = loc[b];

		if ((cmp(key, la->value) == TRUE) || (ll_which->match(la->value, key))) {
			target->head = la;
		}
		if ((cmp(lb->value, key) == TRUE) || (ll_which->match(lb->value, key))) {
			target->tail = lb;
		}

		if (ll_which->match(mid->value, key)) {
			_left_seek(ll_which, loc, key, target, a, x, cmp);
			_right_seek(ll_which, loc, key, target, x, b, cmp);
			break;
		}

		if (cmp(key, mid->value) == TRUE)
			_list_dupbykey(ll_which, loc, key, target, x, b, cmp);
		else
			_list_dupbykey(ll_which, loc, key, target, a, x, cmp);

	} while (0);

	return;
}

list *list_dupByKey(list * ll_which, void *value, int (*cmp) (const void *, const void *))
{
	int len;
	list *duplist=NULL;
	listNode **loc;

    if ( ll_which == NULL ) return(NULL);

	len = listLength(ll_which)-1;
	
	while(len > 0) {

	if ( (loc = ListMap2VectorAddr(ll_which))==NULL ) break;

	if ( ( duplist = ListCreate()) == NULL ) { free(loc); break; }
	duplist->head = duplist->tail = NULL;
	duplist->len = 0;
	duplist->dup = ll_which->dup;
	duplist->free = ll_which->free;
	duplist->match = ll_which->match;

	_list_dupbykey(ll_which, loc, value, duplist, 0, len, cmp);

    free(loc);
    break;
    }

	return (duplist);
}

void _list_bseek_bykey(list * ll_which, listNode ** loc, void *key, listNode *target, int a, int b,
						  int (*bseek_cmp) (const void *, const void *))
{
	int x;
	listNode *la, *lb, *mid;

	do {

		if (a >= b) break;

		x = (a + b) / 2;
		mid = loc[x];
		la = loc[a];
		lb = loc[b];

		if ((a + 1) == b)  {
		
		if ( ll_which->match(key, la->value) == TRUE) {
			target = la;
			break;
		}

		if ( ll_which->match(lb->value, key) == TRUE) {
			target = lb;
			break;
		}

		if ( ll_which->match(mid->value, key) == TRUE ) {
			target = mid;	
			break;
		}
		    target = NULL;
	            break;
		}

		if (bseek_cmp(key, mid->value) == TRUE)
			_list_bseek_bykey(ll_which, loc, key, target, x, b, bseek_cmp);
		else
			_list_bseek_bykey(ll_which, loc, key, target, a, x, bseek_cmp);

	} while (0);

	return;
}

listNode *list_bseek_key(list * ll_which, void *value, int (*cmp) (const void *, const void *))
{
	int len;
	listNode *node_ptr = NULL;
	listNode **loc;

	if ( ( cmp == NULL ) || ( ll_which->match == NULL ) ) return(NULL);

	len = listLength(ll_which) -1;

	while(len>0) {

	if ( (loc = ListMap2VectorAddr(ll_which)) == NULL ) break;
	_list_bseek_bykey(ll_which, loc, value, node_ptr,0, len, cmp);
    free(loc);
        break;
	}

	return (node_ptr);
}

int _list_bDEL_bykey(list * ll_which, listNode ** loc, void *key, int a, int b,
						  int (*bseek_cmp) (const void *, const void *))
{
	int x;
	listNode *la, *lb, *mid;

	do {

		if (a > b)  return(FALSE) ; 

		x = (a + b) / 2;
		mid = loc[x];
		la = loc[a];
		lb = loc[b];

		if ((a + 1) == b)  {
		
		if ( ll_which->match(key, la->value) == TRUE) {
			return(a);
		}

		if ( ll_which->match(mid->value, key) == TRUE ) {
			return(x);
		}
		
 		if ( ( b != ll_which->len ) && ll_which->match(lb->value, key) == TRUE) {
			return(b);
		}

		    return(FALSE);
		}

		if (bseek_cmp(key, mid->value) == TRUE)
			return( _list_bDEL_bykey(ll_which, loc, key, x, b, bseek_cmp) );
		else
			return( _list_bDEL_bykey(ll_which, loc, key, a, x, bseek_cmp) );

	} while (0);

	return(FALSE);
}

void list_bDel_key(list * ll_which, void *value, int (*cmp) (const void *, const void *))
{
	int len,where;

	listNode **loc,*current;

    if ( ll_which == NULL ) return;

	if ( ( cmp == NULL ) || ( ll_which->match == NULL ) ) return;

	len = listLength(ll_which) -1;

	while(len>0) {

	if ( (loc = ListMap2VectorAddr(ll_which)) == NULL ) break;
	if ( (where = _list_bDEL_bykey(ll_which, loc, value, 0, len, cmp) ) == FALSE ) {
            free(loc);
             break;
             }
         
	len = listLength(ll_which) -1;

	if ( where > len ) break;

	current = loc[where]; 
	listDelNode(ll_which, current);
    free(loc);
	}

	return;
}

void list_bDel(list * ll_from, list *ll_to, int (*cmp) (const void *, const void *))
{
	int lenfrom,lento,from,to;

	listNode **loc4,**loc2,*src,*dest;

    if( ( ll_from == NULL ) || ( ll_to == NULL) ) return;

	if ( ( cmp == NULL ) || ( ll_from->match == NULL ) ) return;

	lenfrom = listLength(ll_from) -1;
	lento = listLength(ll_to) -1;

        if(( lenfrom<0) || (lento<0) ) return;

	from = to = 0;

	do {
	if ( (loc4 = ListMap2VectorAddr(ll_from)) == NULL ) break;
	if ( (loc2 = ListMap2VectorAddr(ll_to)) == NULL ) 
                { free(loc4); break; }

	while ( (to <= lento) && ( from <= lenfrom ) && ( lenfrom > 0) ) {
	
	src = loc4[from]; 
	dest = loc2[to]; 

 	if ( ll_from->match(src->value,dest->value) == TRUE) {
	    from++;
	    listDelNode(ll_from, src);
	    continue;
	}

 	if ( cmp(src->value,dest->value) == TRUE) 
	   to++; 
        else  
	   from++;

	}
      
      free(loc2);
      free(loc4);

	} while(0);

	return;
}

void _ListQsort_x(listNode ** loc, int a, int b, int (*cmp) (const void *, const void *,int),int locx)
{

	listNode *la, *lb;
	listNode *tmp;
	int i, x, last;

	if (a >= b)
		return;

	x = (a + b) / 2;
//  _qswap_addr(loc, a, x); 
//  _qswap_nvalues(loc, a, x); 

	tmp = (listNode *)loc[a];
	loc[a] = loc[x];
	loc[x] = tmp;

	last = a;
	la = loc[a];
	for (i = a + 1; i <= b; i++) {
		lb = loc[i];

		if (cmp(la->value, lb->value,locx) == TRUE ) {
	//	_qswap_addr(loc, ++last , i); 
	//	_qswap_nvalues(loc, ++last, i);
	
			last++;
			tmp = (listNode *)loc[last];
			loc[last] = loc[i];
			loc[i] = tmp; 
		}

	}

	//_qswap_addr(loc, a, last); 
	//_qswap_nvalues(loc, a, last);

	tmp = (listNode *)loc[a];
	loc[a] = loc[last];
	loc[last] = tmp;

	_ListQsort_x(loc, a, last - 1, cmp,locx);
	_ListQsort_x(loc, last + 1, b, cmp,locx);

	return;

}

void ListQsort_x(list * ll_which, int (*cmp) (const void *, const void *,int),int locx)
{
	int i = 0, len;
	listNode **loc;
	listNode *first, *target, *chains;

    if ( ll_which == NULL ) return;

	len = listLength(ll_which) - 1;

   while(len > 0) {

	if ( (loc = ListMap2VectorAddr(ll_which)) == NULL ) break;
	
	_ListQsort_x(loc, 0, len, cmp,locx);

	//if swap_nvalues no need regenerate list
	//if qswap_addr uncomment the flow #if

	chains = loc[0];
	first = NULL;

	for (i = 0; i <= len; i++) {
		target = loc[i];

		if (i == 0) {
		  chains = target;
		  chains->prev = NULL;

		} else {

		  first = loc[i - 1];
		  target->prev = first;
		  first->next = target;

		}

	}
	target->next = NULL;

	ll_which->tail = target;
	ll_which->head = chains;

	free(loc);

	break;
   }
	return;
}
#define _MINLEN 16
int append_vnode(vNode *dest,unsigned char *id) {
    int retval = FALSE;    

    do {
       if ( (dest == NULL) || (id==NULL) ) break;
       if ( dest->stackPos >= dest->stackSize ) {
         if ( ( dest->stackPtr = realloc(dest->stackPtr, 
                sizeof(unsigned char *)*(dest->stackSize+_MINLEN) ) ) == NULL ) break;
       }
       dest->stackSize += _MINLEN;
       dest->stackPtr[dest->stackPos++] = id;
     
       retval = TRUE; 
    } while(0); 
    return(retval);    
}

vNode *create_vnode(int xlen) {
    vNode *retval=NULL;
    do {
     if ( xlen < 1 ) break;
     if ( (retval = calloc(1,sizeof(vNode)) ) == NULL ) break;
     retval->stackSize = xlen;
     retval->stackPos = 0;
     retval->stackPtr = calloc(xlen,sizeof(unsigned char *));
    } while(0);
    return(retval);
}

int topmost(vNode *out, void *src, compfunc func) {
    int retval = FALSE;
    int i,j;

    do {
       if ( (out==NULL) || (src==NULL) ) break;
      
       if ( out->stackPos==0 ) { out->stackPtr[out->stackPos++] = src; break; }
       if ( out->stackPos == out->stackSize ) 
         if ( func( (unsigned char *)src,out->stackPtr[out->stackPos-1]) == TRUE ) break;
      
       for( i=0; i<out->stackPos; i++) 
            if ( func( (unsigned char *)src,out->stackPtr[i]) != TRUE ) break;
        
       for( j=out->stackPos-1; j>i; j--) 
            out->stackPtr[j] = out->stackPtr[j-1];
            out->stackPtr[i] = src;

       if ( out->stackPos < out->stackSize-1 ) 
            out->stackPos++;
       
    } while(0);
    return(retval);
}

vNode *extremum(list *l, int n, compfunc func) {
    vNode *vbuff=NULL;
    listIter *iter;
    listNode *node;
    do { 
        if( ( l==NULL ) || ( n < 1 ) || ( func==NULL ) ) break;
        if ( (vbuff=create_vnode(n) ) == NULL ) break;
	    ListEachFromHead(l,iter,node) {
	        topmost(vbuff, (void *)listNodeValue(node), func) ;
	    } ListEachEnd(iter);
    } while(0);
    return(vbuff);
}

