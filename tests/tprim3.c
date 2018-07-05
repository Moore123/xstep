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

#define min(x,y)        ((x) < (y) ? (x) : (y))
#define max(x,y)        ((x) > (y) ? (x) : (y))
 
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
uint64_t initp[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541 };

int main(int argc, char **argv)
{

	int ncount=10,idx=3,ln=10, i=0,opt;
    uint64_t  *p, pdiv,xmax=7, limit=sqrtl( (uint32_t)-1 );
    bool is_primary=TRUE, twins=FALSE;

	struct timeval tm;


    if ( argc < 2 )  { printf("%s -n [] %d\n",argv[0],asizeof(initp)); exit(1); }
	while ((opt = getopt(argc, argv, "n:b:l:t")) != -1) {
		switch (opt) {
		case 'n':
			ncount = atoi(optarg);
            if ( ncount < 10 ) ncount = 10;
            break;
		case 'l':
			ln = atoi(optarg);
            if ( ln < min(ncount,10) ) ln = min(ncount,10);
            break;
		case 't':
            twins = TRUE;
            break;
        default :
            break;
        }
   }
    idx = min(ncount, asizeof(initp));
    p = Calloc(ncount, uint64_t);
    for(i=0; i<idx; i++) p[i] = initp[i];
    pdiv=p[idx-1]+1;
    
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
         p[idx]=pdiv; 
         pdiv += 2;
         idx+=1;
         if ( ( p[idx] - p[idx-1] ) == 2  && (twins == TRUE) ) 
                printf("%d:( %lu %lu ) \n", idx,p[idx-1], p[idx]);
      } else {
         pdiv += 1;
         is_primary = TRUE;
      }
         xmax = sqrtl(pdiv)+1;
         if ( xmax > limit ) break;

    }
 for( idx=ncount-ln ;idx<ncount; idx++)
    printf("%3d: %4lu\n",idx, p[idx]);

	exit(0);
}
