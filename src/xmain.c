
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <ncurses.h>
#include <sys/time.h>

#include "list.h"
#include "kmeans.h"

#ifndef FALSE
#define FALSE 0
#define TRUE (!FALSE)
#define FAILED -1
#define UNCOMPLATE -1
#endif


#define Calloc(n, t)   (t *) calloc( (size_t) (n), sizeof(t) )
#define asizeof(a)     (int)(sizeof (a) / sizeof ((a)[0]))

extern float randf(float max);
int _debug;

typedef float *(*list_fvalue_addr) (void *);

typedef struct _ISOLation {
        uint64_t identify;
        char *stock_name;
        char *stock_datetime;
        uint32_t stk_date_start,stk_date_end;
        uint32_t seq,vvlen,mark,folds;
        float *fv;
} ISOLation;  

inline float *f_isolation_addr(void *param) {
        return((float *)((ISOLation *)param)->fv);
};

inline float *f_node_value(void *param) {
        return((float *)param);
};

inline float **listmap2float_ptr(list * ll_which, list_fvalue_addr get_fvalue_ptr)
{
        int i = 0, j = 0;
        float **loc,*v;
        listNode *node;
        listIter *iter;

        unsigned int align = (listLength(ll_which)+0xF)&0xFFFFFFF0;

        if ((loc = (float **)calloc(align, sizeof(float *))) == NULL)
                   return (NULL);

        ListEachFromHead(ll_which, iter, node) {
            if ( get_fvalue_ptr != NULL )
               loc[i++] = get_fvalue_ptr(listNodeValue(node));
        } ListEachEnd(iter);

     return (loc);
}

void l_kmeans(list *slist,uint32_t width,uint32_t kGroups,double threshold,int loop_iterations, list_fvalue_addr fval_func)
{
	int i, *membership;

	listIter *iter;
	listNode *node;
	struct timeval tm;

	float **xdata;

	while (slist!= NULL) {
        if ( kGroups < 2 ) break;
        if ( listLength(slist) < kGroups ) break;

		i = 0;

		if ((membership = Calloc(listLength(slist)+1, int)) == NULL) {
			break;
        }

       xdata = listmap2float_ptr(slist, fval_func);
    tic(&tm);
	   omp_kmeans(0,xdata, width, (int)listLength(slist), kGroups, threshold, membership);
    ptoc(&tm);
#ifdef GPU
    tic(&tm);
	   cuda_kmeans(xdata, width, (int)listLength(slist), kGroups, threshold, membership, &loop_iterations );
    ptoc(&tm);
#endif        
       i = 0;
       //if (l->func_mark != NULL)
	   ListEachFromHead(slist,iter,node) {
            //l->func_mark( listNodeValue(node) ,&membership[i]);
            i++; 
		} ListEachEnd(iter); 

    free(membership);
	free(xdata);

	break;
  }

	return;
}

int main(int argc, char **argv)
{

	int ncount=100,i,j,opt,width=10,groups=5;
	list *ll;
	float *xval;

    _debug = 0;
    if ( argc < 4 )  { printf("%s -n [N] -w [width] -g [groups] \n",argv[0]); exit(1); }
	while ((opt = getopt(argc, argv, "n:w:g:d")) != -1) {
	   switch (opt) {
		case 'n':
			ncount = atoi(optarg);
            	break;

		case 'w':
			width = atoi(optarg);
            	break;

		case 'g':
			groups = atoi(optarg);
            	break;
        case 'd':
            _debug=1;
            break;

       	default :
            	break;
        }
   }

	if ( ( ll = ListCreate()) != NULL ) {
	 for ( i = 0 ; i < ncount ; i++ ) {
		if ((xval = Calloc(width, float) ) == NULL) continue;
	    for ( j = 0 ; j < width; j++ ) xval[j] = randf(1.0f);
		ListAddNodeTail(ll,xval);

	  }
	}

    l_kmeans(ll,width,groups,0.0000l,100000,f_node_value);

    #ifdef GPU
    fprintf(stderr,"GPU done with listLength is %d\t",listLength(ll));
    #endif
    fprintf(stderr,"OMP done with listLength is %d\n",listLength(ll));
	ListRelease(ll);	
    
	exit(1);

}
