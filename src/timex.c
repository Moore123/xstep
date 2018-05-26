#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define ERR_GENERAL -999
#define Calloc(n, t)   (t *) calloc( (size_t) (n), sizeof(t) )
#define MIN_STR_LEN 64

char *time_string (void) {

  struct timeval tv;
  gettimeofday (&tv, NULL);
  struct tm tm;
 
  char *time_buf = NULL;

  if (localtime_r (&tv.tv_sec, &tm) == NULL)
    {
      fprintf (stderr, "ERROR: in getting time\n");
      exit (ERR_GENERAL);
    }
  do {
    if ( ( time_buf = Calloc( MIN_STR_LEN, char) ) == NULL ) break;
    sprintf (time_buf, "%02d/%02d/%04d %02d:%02d:%02d",
	   tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900, tm.tm_hour,
	   tm.tm_min, tm.tm_sec);
   } while(0);

  return time_buf;
}

/* Subtracts time values to determine run time */
int timeval_subtract (struct timeval *result, struct timeval *t2, struct timeval *t1)
{
  long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
  result->tv_sec = diff / 1000000;
  result->tv_usec = diff % 1000000;

  return (diff < 0);
}


/* Starts timer */
void tic (struct timeval *timer) { 
    memset(timer,0x0,sizeof(struct timeval));
	gettimeofday(timer, NULL); 
}


/* Stops timer and prints difference to the screen */
void toc (struct timeval *timer) {
  struct timeval tv_end, tv_diff;

  memset(&tv_end,0x0,sizeof(struct timeval));
  memset(&tv_diff,0x0,sizeof(struct timeval));
  gettimeofday(&tv_end, NULL);
  timeval_subtract (&tv_diff, &tv_end, timer);
  //ndebug(" running time is %ld.%06ld\n", tv_diff.tv_sec, tv_diff.tv_usec);

}

/* Stops timer and prints difference to the screen */
void ptoc (struct timeval *timer) {
  struct timeval tv_end, tv_diff;

  memset(&tv_end,0x0,sizeof(struct timeval));
  memset(&tv_diff,0x0,sizeof(struct timeval));
  gettimeofday(&tv_end, NULL);
  timeval_subtract (&tv_diff, &tv_end, timer);
  fprintf(stderr,"\n %s running time is %ld.%06ld\n",__func__, tv_diff.tv_sec, tv_diff.tv_usec);
}


