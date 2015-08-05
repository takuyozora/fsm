//
// Created by olivier on 05/08/15.
//

#include "time.h"
#include <sys/time.h>
#include <sys/resource.h>

double bm_get_time()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + t.tv_usec*1e-6;
}

double bm_get_clock()
{
    return (double)clock()/CLOCKS_PER_SEC;
}


