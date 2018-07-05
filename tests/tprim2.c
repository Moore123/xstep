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

#define TRUE 1
#define FALSE 0
        
#define Calloc(n, t)   (t *) calloc( (size_t) (n), sizeof(t) )
#define asizeof(a)     (int)(sizeof (a) / sizeof ((a)[0]))
 
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

int main(int argc, char **argv)
{

	int ncount=10,idx=3, i=0,opt;
    uint64_t  *p, pdiv,xmax=7, limit=sqrtl( (uint32_t)-1 );
    bool is_primary=TRUE, twins=FALSE;

	struct timeval tm;


    if ( argc < 2 )  { printf("%s -n [] \n",argv[0]); exit(1); }
	while ((opt = getopt(argc, argv, "n:b:t")) != -1) {
		switch (opt) {
		case 'n':
			ncount = atoi(optarg);
            if ( ncount < 10 ) ncount = 10;
            break;
		case 't':
            twins = TRUE;
            break;
        default :
            break;
        }
   }
    p = Calloc(ncount, uint64_t);
    p[0]=2;
    p[1]=3;
    p[2]=5;
    p[3]=7;
    p[4]=11;
    pdiv=p[idx]+1;
    
    while( idx < ncount ) { 
//    #pragma omp parallel for
    for( i=0; i<=idx; i++) {
      //if( unlikely( is_primary == FALSE ) ) continue;
      //if( is_primary == FALSE ) continue;
      // if ( p[i] > ( pdiv>>1) ) continue;
      if ( p[i] > xmax  ) break;
      if ( ( pdiv % p[i] ) == 0 ) {
        is_primary = FALSE;
        break;
      }
     }

     if ( is_primary == TRUE ) {
         idx+=1;
         p[idx]=pdiv; 
         pdiv += 2;
         if ( ( p[idx] - p[idx-1] ) == 2  && (twins == TRUE) ) 
                printf("%d:( %lu %lu ) \n", idx,p[idx-1], p[idx]);
      } else {
         pdiv += 1;
         is_primary = TRUE;
      }
         xmax = sqrtl(pdiv)+1;
         if ( xmax > limit ) break;

    }
 for( idx=ncount-10 ;idx<ncount; idx++)
    printf("%3d: %4lu\n",idx, p[idx]);

	exit(0);
}
