//
// Created by olivier on 08/08/15.
//


#include <stdlib.h>
#include "fsm_time.h"
#include "fsm_debug.h"

#ifdef DBG_TEST_EXE
#include "../test/wrapper.h"
#endif



struct timespec fsm_time_get_abs_fixed_time_from_us(int delta_us) {
        struct timespec ts;
        check(clock_gettime(FSM_CLOCK_MONOTONIC_SOURCE, &ts)==0, "CRITICAL : Impossible to get boot time : abort");
//        if(clock_gettime(CLOCK_BOOTTIME, &ts) != 0){
//            log_warn("IMPORTANT : Impossible to get boot time : try with monotonic_raw");
//            #ifdef DBG_TEST_EXE
//            check(0, "Test stop here"); // No root : bug if clock_gettime is called a second time cause of mocka wrapper engine
//            #endif
//            check(clock_gettime(CLOCK_MONOTONIC_RAW, &ts)==0, "CRITICAL : Impossible to get monotonic_raw time : abort");
//        }
        ts.tv_sec += delta_us / 1000000;
        ts.tv_nsec +=  1000 * ( delta_us % 1000 );
        ts.tv_sec += ts.tv_nsec / FSM_TIME_NANO_SECONDE;
        ts.tv_nsec %= FSM_TIME_NANO_SECONDE;
        return ts;
    error:
        dbg_test_exe(ts.tv_nsec = 0; ts.tv_sec = 0;)
        log_warn("fsm_time_get_abs_fixed_time_from_us return 0,0 because of test_exe ");
        //exit(-1);
        return ts;
}

int fsm_time_delta_ns(struct timespec t_start, struct timespec t_end) {
    int delta_s = (int)(t_end.tv_sec - t_start.tv_sec);
    if (delta_s > FSM_TIME_MAX_INT_NANO_SECONDE){
        // Overflow we return maximum value
        return INT_MAX;
    } else if (delta_s < -FSM_TIME_NANO_SECONDE){
        // Overflow we return minimum value
        return -INT_MAX;
    }
    delta_s *= FSM_TIME_NANO_SECONDE; // Now represent nanosecond
    int delta_ns = (int)(t_end.tv_nsec - t_start.tv_nsec);
    if(delta_ns > 0){
        if(delta_ns > INT_MAX - delta_s){
            // Overflow we return maximum value
            return INT_MAX;
        }
    }else{
        if(-delta_ns > INT_MAX + delta_s){
            // Overflow we return minimum value
            return -INT_MAX;
        }
    }
    return delta_ns + delta_s; // Should be protected from overflow !
}

bool fsm_time_check_absolute_time(struct timespec ts){
    return fsm_time_delta_ns(fsm_time_get_abs_fixed_time_from_us(0), ts) > 0;
}