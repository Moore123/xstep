/* list.h - A generic doubly linked list implementation
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <pthread.h>
#include <ck_rwlock.h>

#ifndef __DBListIST_H__
#define __DBListIST_H__

#define LL_HIDDEN __attribute__ ((visibility ("hidden")))

/* Node, List, and Iterator are the only data structures used currently. */

#define TRUE 1
#define FALSE 0
#define UNCOMPLATE -1

#define PARALLELORNOT 150000

/* red-black tree description */
typedef enum { Black, Red, Gray, White, Pinky } NodeColor;

typedef enum {
	LL_INCR = 0,
	LL_DECR,
	LL_RAW
} issorted;

typedef struct listNode {
	struct listNode *prev;
	struct listNode *next;
	void *value;
} listNode;

typedef struct listIter {
	listNode *next;
	int direction;
} listIter;

typedef struct list {
	listNode *head;
	listNode *tail;

	issorted sorted;

	ck_rwlock_t entity_lock;
	pthread_mutex_t mutex_lock;
	pthread_cond_t  cwait;
	int sort_threads;

	void *(*dup) (void *ptr);
	void (*free) (void *ptr);
	int (*match) (const void *ptr, const void *key);
	int (*vsize) (const void *ptr);

	unsigned int len;
} list;

/* Functions implemented as macros */
#define listLength(l) ((l)->len)

#define listFirst(l) ((l)->head)
#define listLast(l) ((l)->tail)
#define listPrevNode(n) ((n)->prev)
#define listNextNode(n) ((n)->next)
#define listNodeValue(n) ((n)->value)

#define listHeadNodeValue(n) ((n)->head->value)
#define listTailNodeValue(n) ((n)->tail->value)

#define listSetDupMethod(l,m) ((l)->dup = (m))
#define listSetFreeMethod(l,m) ((l)->free = (m))
#define listSetMatchMethod(l,m) ((l)->match = (m))

#define listGetDupMethod(l) ((l)->dup)
#define listGetFree(l) ((l)->free)
#define listGetMatchMethod(l) ((l)->match)

/* Prototypes */
extern list *ListCreate(void);
extern void ListRelease(list * list);
extern list *ListAddNodeHead(list * list, void *value);
extern list *ListAddNodeTail(list * list, void *value);
extern list *listInsertNode(list * list, listNode * old_node, void *value, int after);
extern void listDelNode(list * list, listNode * node);
extern listIter *listGetIterator(list * list, int direction);
extern listNode *ListNext(listIter * iter);
extern void ListReleaseIterator(listIter * iter);
extern list *ListDup(list * orig);
extern listNode *ListSearchKey(list * list, void *key);
extern listNode *ListIndex(list * list, long index);
extern void ListRewind(list * list, listIter * li);
extern void ListRewindTail(list * list, listIter * li);
extern void ListRotate(list * list);
extern void ListMerge(list *, list *);
extern void ListUnique(list *, int (*equ) (const void *, const void *),
				int (*merge)(const void *,const void *) );
extern void ListNeiUnique(list *, int (*equ) (const void *, const void *),
				int (*merge)(const void *,const void *) );

extern void ListSort(list *, int (*cmp) (const void *, const void *));
extern void listQuickSort(listNode *, listNode *, int (*cmp) (const void *, const void *));

/* item less then 100k ,should be used qsort */

extern void ListQsort(list *, int (*cmp) (const void *, const void *));

extern void ListQsort_x(list *, int (*cmp) (const void *, const void *,int),int);

/* item great then 100k ,should be used psort */
extern void list_psort(list *, int (*cmp) (const void *, const void *));

extern void list_binsert(list *, void *, int (*cmp) (const void *, const void *));
extern void list_bDel_key(list *, void *, int (*cmp) (const void *, const void *));
extern void list_bDel(list *, list *, int (*cmp) (const void *, const void *));

extern list *listDupByKey(list *, void *, int (*cmp) (const void *, const void *));

extern listNode *list_bseek_key(list *, void *, int (*cmp) (const void *, const void *));
extern listNode **ListMap2VectorAddr(list *) ;

/* Directions for iterators */
#define AL_START_HEAD 0
#define AL_START_TAIL 1

#define ListEachFromHead(LIST,ITER,NODE) \
        ITER=listGetIterator(LIST,AL_START_HEAD); \
	    while ((NODE = ListNext(ITER)) != NULL) 

#define ListEachFromTail(LIST,ITER,NODE) \
        ITER=listGetIterator(LIST,AL_START_TAIL); \
	    while ((NODE = ListNext(ITER)) != NULL) 

#define ListEachEnd(ITER)  \
        ListReleaseIterator(ITER);

#define wListEachFromHead(LIST,ITER,NODE) \
	    while( ck_rwlock_write_trylock(&LIST->entity_lock) != 0 ) ;; \
        ITER=listGetIterator(LIST,AL_START_HEAD); \
	    while ((NODE = ListNext(ITER)) != NULL) 

#define wListEachFromTail(LIST,ITER,NODE) \
	    while( ck_rwlock_write_trylock(&LIST->entity_lock) != 0 ) ;; \
        ITER=listGetIterator(LIST,AL_START_TAIL); \
	    while ((NODE = ListNext(ITER)) != NULL) 

#define rListEachFromHead(LIST,ITER,NODE) \
	    while( ck_rwlock_read_trylock(&LIST->entity_lock) != 0 ) ;; \
        ITER=listGetIterator(LIST,AL_START_HEAD); \
	    while ((NODE = ListNext(ITER)) != NULL) 

#define rListEachFromTail(LIST,ITER,NODE) \
	    while( ck_rwlock_read_trylock(LIST->entity_lock) != 0 ) ;; \
        ITER=listGetIterator(LIST,AL_START_TAIL); \
	    while ((NODE = ListNext(ITER)) != NULL) 

#define rListEachEnd(LIST,ITER)  \
	    ck_rwlock_read_unlock(&LIST->entity_lock); \
        ListReleaseIterator(ITER);

#define wListEachEnd(LIST,ITER)  \
	    ck_rwlock_write_unlock(&LIST->entity_lock); \
        ListReleaseIterator(ITER);


#define ListWriteHead(LIST,NODE) \
	    while( ck_rwlock_write_trylock(&LIST->entity_lock) != 0 ) ;; \
        ListAddNodeHead(LIST,NODE); \
        list_write_unlock(LIST);

#define ListWriteTail(LIST,NODE) \
	    while( ck_rwlock_write_trylock(&LIST->entity_lock) != 0 ) ;; \
        ListAddNodeTail(LIST,NODE); \
        list_write_unlock(LIST);

#define list_wrlock(LIST) \
	    ck_rwlock_write_lock(LIST->entity_lock)

#define list_rdlock(LIST) \
	    ck_rwlock_read_lock(LIST->entity_lock)

#define list_trywrlock(LIST) \
	    while( ck_rwlock_write_trylock(&LIST->entity_lock) != 0 ) ;; \

#define list_tryrdlock(LIST) \
	    while( ck_rwlock_read_tryrdlock(LIST->entity_lock) != 0 ) ;; \

#define else_list_lock(LIST) \
        else {  \
	    ck_rwlock_unlock(&LIST->entity_lock); \
        continue; }

#define list_read_unlock(LIST) \
	    ck_rwlock_read_unlock(&LIST->entity_lock) 

#define list_write_unlock(LIST) \
	    ck_rwlock_write_unlock(&LIST->entity_lock) 
	   

typedef struct _namedList {
        char *cName;
		unsigned int mark;
        list *l;
} namedList;

typedef struct  __vNode {
    int stackSize;    
    int stackPos;     
	void (*free) (void *ptr);
    unsigned char **stackPtr; 
} vNode;

extern list *getNamedList(list *, char *);
extern list *addNamedListHead(list *, char *, void *);
extern list *addNamedListTail(list *, char *, void *);
extern void namedListReleaseItem(list *, char *);
extern void freeNamedList(void *);

typedef int (*compfunc)(unsigned char *a, unsigned char *b);

extern int append_vnode(vNode *,unsigned char *);
extern vNode *create_vnode(int);
extern vNode *extremum(list *, int, compfunc);
#endif	  /* __DBListIST_H__ */
