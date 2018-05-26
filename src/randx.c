#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>

/*

 thread safe rand 
 return int

*/

#ifndef FALSE
#define FALSE 0
#define TRUE (!FALSE)
#define FAILED -1
#define UNCOMPLATE -1
#endif


int randi(int max)
{
    int rand_num;
    static struct random_data rand_data;
    static char rand_state[256];
    static bool initialized = FALSE;

    if (!initialized)
    {
        initstate_r(time(0), rand_state, sizeof(rand_state), &rand_data);
        initialized =  TRUE;
    }
    random_r(&rand_data, &rand_num);
    double cof = rand_num / (RAND_MAX + 1.0);
    return (int)(max * cof);
}

int32_t randl(uint32_t max)
{
    int32_t rand_num;
    static struct random_data rand_data;
    static char rand_state[256];
    static bool initialized = FALSE;

    if (!initialized)
    {
        initstate_r(time(0), rand_state, sizeof(rand_state), &rand_data);
        initialized =  TRUE;
    }
    random_r(&rand_data, &rand_num);
    double cof = rand_num / (RAND_MAX + 1.0);
    return (uint32_t)(max * cof);
}

float randf(float max)
{
    int rand_num;
    static struct random_data rand_data;
    static char rand_state[256];
    static bool initialized = FALSE;

    if (!initialized)
    {
        initstate_r(time(0), rand_state, sizeof(rand_state), &rand_data);
        initialized =  TRUE;
    }
    random_r(&rand_data, &rand_num);
    double cof = rand_num / (RAND_MAX + 1.0);
    return (float)(max * cof);
}

double randd(double max)
{
	int rand_num;
	static struct random_data rand_data;
	static char rand_state[256];
        static bool initialized = FALSE;

	if (!initialized) {
		initstate_r(time(0), rand_state, sizeof(rand_state), &rand_data);
		initialized = 1;
	}
	random_r(&rand_data, &rand_num);
	double cof = rand_num / (RAND_MAX + 1.0);
	return (max * cof);
}

