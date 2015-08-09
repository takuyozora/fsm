//
// Created by olivier on 04/08/15.
//

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <unistd.h>
#include "pthread.h"
#include <stdio.h>

#include "fsm_debug.h"
#include "fsm_time.h"


void test_time_delta_overflow(void **state){
    struct timespec t_s_start = {
            .tv_sec = 4,
            .tv_nsec = 0,
    };
    struct timespec t_s_end = {
            .tv_sec = t_s_start.tv_sec+ 1 + FSM_TIME_MAX_INT_NANO_SECONDE,
            .tv_nsec = 0,
    };
    assert_int_equal(fsm_time_delta_ns(t_s_start, t_s_end), INT_MAX);
    assert_int_equal(fsm_time_delta_ns(t_s_end, t_s_start), -INT_MAX);
    t_s_end.tv_sec -= 1;
    assert_int_not_equal(fsm_time_delta_ns(t_s_start, t_s_end), INT_MAX);
    assert_int_not_equal(fsm_time_delta_ns(t_s_end, t_s_start), -INT_MAX);
    t_s_end.tv_nsec = FSM_TIME_NANO_SECONDE;
    assert_int_equal(fsm_time_delta_ns(t_s_start, t_s_end), INT_MAX);
    assert_int_equal(fsm_time_delta_ns(t_s_end, t_s_start), -INT_MAX);
    t_s_start.tv_nsec = FSM_TIME_NANO_SECONDE;
    assert_int_not_equal(fsm_time_delta_ns(t_s_start, t_s_end), INT_MAX);
    assert_int_not_equal(fsm_time_delta_ns(t_s_end, t_s_start), -INT_MAX);

}

void test_time_delta(void **state){
    struct timespec ts = {
            .tv_sec = 4,
            .tv_nsec = 0,
    };
    struct timespec ts2 = {
            .tv_sec = ts.tv_sec+ 1,
            .tv_nsec = 100,
    };
    assert_int_equal(fsm_time_delta_ns(ts2, ts), -fsm_time_delta_ns(ts, ts2));
}

void test_time_get_abs_fixed_time_from_us(void **state){
    will_return(__wrap_clock_gettime, -1);
    struct timespec ts = fsm_time_get_abs_fixed_time_from_us(1000);
    assert_int_equal(ts.tv_sec, 0);
    assert_int_equal(ts.tv_nsec, 0);
    will_return(__wrap_clock_gettime, 0);
    ts = fsm_time_get_abs_fixed_time_from_us(1000);
    assert_int_not_equal(ts.tv_sec, 0);
    assert_int_not_equal(ts.tv_nsec, 0);
}

void test_time_check_absolute_time(void **state){
    will_return(__wrap_clock_gettime, 0);
    struct timespec ts = fsm_time_get_abs_fixed_time_from_us(-1);
    will_return(__wrap_clock_gettime, 0);
    assert_true(!fsm_time_check_absolute_time(ts));
    will_return(__wrap_clock_gettime, 0);
    ts = fsm_time_get_abs_fixed_time_from_us(INT_MAX);
    will_return(__wrap_clock_gettime, 0);
    assert_true(fsm_time_check_absolute_time(ts));
}


int main(void)
{
    srand ((unsigned int) time(NULL));
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(test_time_delta_overflow),
            cmocka_unit_test(test_time_delta),
            cmocka_unit_test(test_time_get_abs_fixed_time_from_us),
            cmocka_unit_test(test_time_check_absolute_time),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}