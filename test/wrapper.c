//
// Created by olivier on 09/08/15.
//


#include "wrapper.h"

int __wrap_clock_gettime (clockid_t clk_id, struct timespec *tp){
    int rc = (int) mock();
    if (rc == -1) {
        tp->tv_sec = 0;
        tp->tv_nsec = 0;
        return rc;
    }
    return __real_clock_gettime(clk_id, tp);
    //return rc;
}

