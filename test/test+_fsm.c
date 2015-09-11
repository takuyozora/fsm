//
// Created by olivier on 07/08/15.
//

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <unistd.h>
#include "pthread.h"
#include <stdio.h>


#include "fsm.h"
#define NDEBUG
#include "fsm_debug.h"
#include "benchmark.h"

#define MAX_INCREMENT_CALLBACK 100000

void *callback_set_int_from_step_to_42(struct fsm_context *context){
    *(int *)context->pointer->current_step->args = 42;
    return NULL;
}

void *callback_assert_int_from_event_is_42(struct fsm_context *context){
    assert_int_equal(*(int *)context->event->args, 42);
    return NULL;
}

void *callback_return_step_from_event(struct fsm_context *context){
    return (struct fsm_step *)context->event->args;
}

void *callback_increment_int_from_step(struct fsm_context *context){
    if (*(int *)context->pointer->current_step->args < MAX_INCREMENT_CALLBACK) {
        (*(int *) context->pointer->current_step->args)++;
    }else{
        fsm_queue_cleanup(context->pointer->current_step->transitions);
    }
    return NULL;
}

struct create_fsm{
    struct fsm_pointer **pointer;
    int *data;
};

void *callback_create_fsm(struct fsm_context *context){
    struct create_fsm *data =((struct create_fsm *)context->pointer->current_step->args);
    *data->pointer = fsm_create_pointer();
    struct fsm_step *step0 = fsm_create_step(fsm_null_callback, NULL);
    struct fsm_step *step1 = fsm_create_step(callback_set_int_from_step_to_42, data->data);
    fsm_connect_step(step0, step1, "GO");
    fsm_start_pointer(*data->pointer, step0);
    return NULL;
}

void benchmark_fsm_direct_transitions(void **state){
    struct fsm_pointer *fsm = fsm_create_pointer();
    int i = 0;
    struct fsm_step *step_0 = fsm_create_step(callback_increment_int_from_step, (void *) &i);
    struct fsm_step *step_1 = fsm_create_step(callback_increment_int_from_step, (void *) &i);

    fsm_connect_step(step_0, step_1, _EVENT_DIRECT_TRANSITION);
    fsm_connect_step(step_1, step_0, _EVENT_DIRECT_TRANSITION);

    double start_time = bm_get_clock();
    fsm_start_pointer(fsm, step_0);
    fsm_join_pointer(fsm);
    double diff_time = bm_get_clock() - start_time;

    log_info("Benchmark for %u direct step transition : %f s", MAX_INCREMENT_CALLBACK, diff_time);
    log_info("Benchmark for 1 direct step transition : ~%f ns", diff_time*1000000000/MAX_INCREMENT_CALLBACK);

    fsm_join_pointer(fsm);
    fsm_delete_pointer(fsm);
    fsm_delete_all_steps();
}

void benchmark_fsm_ping_pong_transitions(void **state){
    struct fsm_pointer *fsm = fsm_create_pointer();
    int i = 0;
    struct fsm_step *step_0 = fsm_create_step(callback_increment_int_from_step, (void *) &i);
    struct fsm_step *step_1 = fsm_create_step(callback_increment_int_from_step, (void *) &i);

    fsm_connect_step(step_0, step_1, "NEXT");
    fsm_connect_step(step_1, step_0, "NEXT");

    double start_time = bm_get_clock();
    fsm_start_pointer(fsm, step_0);
    for(int i=0; i < MAX_INCREMENT_CALLBACK; i++){
        fsm_signal_pointer_of_event(fsm, fsm_generate_event("NEXT", NULL));
        debug("i = %d", i);
        debug("step == step0 : %d", fsm->current_step==step_0);
        debug("step == step1 : %d", fsm->current_step==step_1);
        /*if(i%2 == 1) {
            fsm_wait_step_blocking(fsm, step_0);
        }else{
            fsm_wait_step_blocking(fsm, step_1);
        }*/
        debug("Unlock..");
    }
    debug("Join");
    fsm_join_pointer(fsm);
    double diff_time = bm_get_clock() - start_time;

    log_info("Benchmark for %u step transition : %f s", MAX_INCREMENT_CALLBACK, diff_time);
    log_info("Benchmark for 1 step transition : ~%f ns", diff_time*1000000000/MAX_INCREMENT_CALLBACK);

    fsm_join_pointer(fsm);
    fsm_delete_pointer(fsm);
    fsm_delete_all_steps();
}

void test_fsm_break_direct_loop(void **state){
    struct fsm_pointer *fsm = fsm_create_pointer();
    struct fsm_step *step_0 = fsm_create_step(fsm_null_callback, NULL);
    struct fsm_step *step_1 = fsm_create_step(fsm_null_callback, NULL);

    fsm_connect_step(step_0, step_1, _EVENT_DIRECT_TRANSITION);
    fsm_connect_step(step_1, step_0, _EVENT_DIRECT_TRANSITION);

    fsm_start_pointer(fsm, step_0);
    assert_int_equal(fsm_wait_step_mstimeout(fsm, step_1, 100), 0);
    fsm_join_pointer(fsm);
    assert_int_equal(fsm->running, FSM_STATE_STOPPED);
    fsm_delete_pointer(fsm);
    fsm_delete_all_steps();
}

int main(void)
{
    srand ((unsigned int) time(NULL));
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(test_fsm_break_direct_loop),
            cmocka_unit_test(benchmark_fsm_direct_transitions),
            cmocka_unit_test(benchmark_fsm_ping_pong_transitions),
    };

    int rc = cmocka_run_group_tests(tests, NULL, NULL);
    return rc;
}