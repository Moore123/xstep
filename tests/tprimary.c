#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include <ncurses.h>
#include <sys/time.h>
#include "list.h"


#define TRUE 1
#define FALSE 0

#define Calloc(n, t)   (t *) calloc( (size_t) (n), sizeof(t) )
#define asizeof(a)     (int)(sizeof (a) / sizeof ((a)[0]))

int main(int argc, char **argv)
{

	int ncount=10,i=0, j=0,opt,x;
    uint64_t p1=3, *pdiv, *p, *ptail;
    bool is_primary=TRUE;
    list *xp;
    listIter *iter;
    listNode *node;

	struct timeval tm;

    if ( argc < 2 )  { printf("%s -n [] \n",argv[0]); exit(1); }
	while ((opt = getopt(argc, argv, "n:b:")) != -1) {
		switch (opt) {
		case 'n':
			ncount = atoi(optarg);
            break;
        default :
            break;
        }
   }
    xp = ListCreate();
    xp->free = free;
    p = Calloc(1, uint64_t);
    pdiv = Calloc(1, uint64_t);
    *p=2;
    ListAddNodeTail(xp,p); 
    pdiv = listTailNodeValue(xp);
    p = Calloc(1, uint64_t);
    *p = *pdiv +1; 
     
    while( i < ncount ) {

     ListEachFromTail(xp,iter,node) {
       pdiv = (uint64_t *)(listNodeValue(node));
       if( ( *p % (*pdiv) ) == 0  ) is_primary = FALSE;
     } ListEachEnd(iter);

     if ( is_primary == TRUE ) {
         ListAddNodeTail(xp,p);
         if( !( i % 1000 ) ) printf( "%8d: %lu \n",i, *p );
         p = Calloc(1, uint64_t);
         pdiv = listTailNodeValue(xp); 
         *p = *pdiv + 1;
         i++;
      } else {
         *p = *p+1;
         is_primary = TRUE;
      }

    }

	exit(0);
}
