//
// Created by olivier on 09/08/15.

#ifndef FSM_WRAPPER_H
#define FSM_WRAPPER_H

#define DBG_TEST_EXE
#define DBG_TEST


#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "stdbool.h"

#include "fsm_debug.h"
#include "time.h"

int __wrap_clock_gettime (clockid_t clk_id, struct timespec *tp);

#endif //FSM_WRAPPER_H
