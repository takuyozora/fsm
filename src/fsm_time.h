//
// Created by olivier on 08/08/15.
//

#ifndef FSM_TIMING_H
#define FSM_TIMING_H

#include "time.h"
#include "bits/time.h"
#include "limits.h"
#include "stdbool.h"

#define FSM_TIME_NANO_SECONDE 1000000000
#define FSM_TIME_MAX_INT_NANO_SECONDE INT_MAX / 1000000000
#define FSM_CLOCK_MONOTONIC_SOURCE 1 // CLOCK_MONOTONIC_RAW

struct timespec fsm_time_get_abs_fixed_time_from_us(int delta_us);

int fsm_time_delta_ns(struct timespec t_start, struct timespec t_end);

bool fsm_time_check_absolute_time(struct timespec ts);

#endif //FSM_TIMING_H

