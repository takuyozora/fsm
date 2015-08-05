//
// Created by olivier on 04/08/15.
//

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "fsm.h"
#include "pthread.h"
#include "debug.h"

void *callback_set_int_from_step_to_42(const struct fsm_context *context){
    *(int *)context->fnct_args = 42;
    return NULL;
}

void *callback_assert_int_from_event_is_42(const struct fsm_context *context){
    assert_int_equal(*(int *)context->event->args, 42);
    return NULL;
}

void *callback_return_step_from_event(const struct fsm_context *context){
    return (struct fsm_step *)context->event->args;
}

void test_fsm_start_stop(void **state){
    struct fsm_pointer *fsm = create_pointer();
    struct fsm_step *step_0 = create_step(fsm_null_callback, NULL);
    start_pointer(fsm, step_0);
    assert_int_equal(fsm->running, 1);
    join_pointer(fsm);
    assert_int_equal(fsm->running, 0);
    destroy_pointer(fsm);
    destroy_all_steps();
}

void test_fsm_rand_transition(void **state){
    struct fsm_pointer *fsm = create_pointer();
    struct fsm_step *step_0 = create_step(fsm_null_callback, NULL);
    struct fsm_step *step_1 = create_step(fsm_null_callback, NULL);
    struct fsm_step *step_2 = create_step(fsm_null_callback, NULL);

    connect_step(step_0, step_1, "GO_TO_1");
    connect_step(step_0, step_2, "GO_TO_2");

    char passed_by = 1; // Ensure we have test step 1 and 2 using primary numbers

    while(passed_by % 2 != 0 && passed_by % 3 != 0){
        start_pointer(fsm, step_0);

        char choice = rand() % 2 == 1;
        if (choice) {
            signal_fsm_pointer_of_event(fsm, generate_event("GO_TO_1", NULL));
        } else {
            signal_fsm_pointer_of_event(fsm, generate_event("GO_TO_2", NULL));
        }

        join_pointer(fsm);
        assert_ptr_not_equal(fsm->current_step, step_0);
        if (choice) {
            assert_ptr_equal(fsm->current_step, step_1);
            passed_by *= 2;
        } else {
            assert_ptr_equal(fsm->current_step, step_2);
            passed_by *= 3;
        }
    }

    destroy_pointer(fsm);
    destroy_all_steps();
}

void test_fsm_passing_value_by_step(void **step){
    struct fsm_pointer *fsm = create_pointer();
    int value = 5;
    struct fsm_step *step_0 = create_step(callback_set_int_from_step_to_42, (void *) &value);
    start_pointer(fsm, step_0);
    join_pointer(fsm);
    destroy_pointer(fsm);
    destroy_all_steps();
    assert_int_equal(value, 42);
}

void test_fsm_passing_value_by_event(void **step){
    struct fsm_pointer *fsm = create_pointer();
    int value = 42;
    struct fsm_step *step_0 = create_step(fsm_null_callback, NULL);
    struct fsm_step *step_1 = create_step(callback_assert_int_from_event_is_42, NULL);

    connect_step(step_0, step_1, "TEST_PASSING_VALUE");

    start_pointer(fsm, step_0);

    signal_fsm_pointer_of_event(fsm, generate_event("TEST_PASSING_VALUE", (void *)&value));

    join_pointer(fsm);
    assert_ptr_equal(fsm->current_step, step_1);
    destroy_pointer(fsm);
    destroy_all_steps();
}

void test_fsm_change_step_by_callback_return(void **state){
    struct fsm_pointer *fsm = create_pointer();
    struct fsm_step *step_0 = create_step(fsm_null_callback, NULL);
    struct fsm_step *step_1 = create_step(callback_return_step_from_event, NULL);
    struct fsm_step *step_2 = create_step(fsm_null_callback, NULL);
    struct fsm_step *step_3 = create_step(fsm_null_callback, NULL);


    connect_step(step_0, step_1, "TEST");
    connect_step(step_2, step_0, "RETURN");
    connect_step(step_3, step_0, "RETURN");

    start_pointer(fsm, step_0);

    signal_fsm_pointer_of_event(fsm, generate_event("TEST", (void *)step_2));
    pthread_mutex_lock(&fsm->mutex);
    while(fsm->current_step == step_1 || fsm->current_step == step_0){
        pthread_cond_wait(&fsm->cond_event, &fsm->mutex);
    }
    pthread_mutex_unlock(&fsm->mutex);
    assert_ptr_equal(step_2, fsm->current_step);
    signal_fsm_pointer_of_event(fsm, generate_event("RETURN", NULL));
    pthread_mutex_lock(&fsm->mutex);
    while(fsm->current_step != step_0){
        pthread_cond_wait(&fsm->cond_event, &fsm->mutex);
    }
    pthread_mutex_unlock(&fsm->mutex);
    signal_fsm_pointer_of_event(fsm, generate_event("TEST", (void *)step_3));
    pthread_mutex_lock(&fsm->mutex);
    while(fsm->current_step != step_3){
        pthread_cond_wait(&fsm->cond_event, &fsm->mutex);
    }
    pthread_mutex_unlock(&fsm->mutex);
    assert_ptr_equal(step_3, fsm->current_step);
    signal_fsm_pointer_of_event(fsm, generate_event("RETURN", NULL));

    join_pointer(fsm);
    destroy_pointer(fsm);
    destroy_all_steps();
}

void test_fsm_direct_transition(void **state){
    struct fsm_pointer *fsm = create_pointer();
    struct fsm_step *step_0 = create_step(fsm_null_callback, NULL);
    struct fsm_step *step_1 = create_step(fsm_null_callback, NULL);

    connect_step(step_0, step_1, _EVENT_DIRECT_TRANSITION);

    start_pointer(fsm, step_0);
    assert_int_equal(fsm_wait_step_mstimeout(fsm, step_1, 1000), 0);

    join_pointer(fsm);
    destroy_pointer(fsm);
    destroy_all_steps();
}

int main(void)
{
    srand ((unsigned int) time(NULL));
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(test_fsm_start_stop),
            cmocka_unit_test(test_fsm_rand_transition),
            cmocka_unit_test(test_fsm_passing_value_by_step),
            cmocka_unit_test(test_fsm_passing_value_by_event),
            cmocka_unit_test(test_fsm_change_step_by_callback_return),
            cmocka_unit_test(test_fsm_direct_transition),
    };

    int rc = cmocka_run_group_tests(tests, NULL, NULL);
    return rc;
}