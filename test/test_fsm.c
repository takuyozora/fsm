//
// Created by olivier on 04/08/15.
//

//#define DBG_VERBOSE

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <unistd.h>
#include "pthread.h"
#include <stdio.h>


#include "fsm.h"
//#define NDEBUG
#include "fsm_debug.h"

#define MAX_INCREMENT_CALLBACK 100000
#define AVG_WAIT_STEP_TIMEOUT_MS 500

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

struct fsm_step *callback_pointer_step_1 = NULL;
struct fsm_step *callback_pointer_step_2 = NULL;

void *callback_condtrans_step1_or_2(struct fsm_context *context){
    if(*(int *)context->event->args == 1){
        return callback_pointer_step_1;
    }else if (*(int *)context->event->args == 2){
        return callback_pointer_step_2;
    }else{
        return NULL;
    }
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

void test_fsm_start_stop(void **state){
    struct fsm_pointer *fsm = fsm_create_pointer();
    struct fsm_step *step_0 = fsm_create_step(fsm_null_callback, NULL);
    fsm_start_pointer(fsm, step_0);
    assert_int_equal(fsm->running, FSM_STATE_RUNNING);
    fsm_join_pointer(fsm);
    assert_int_equal(fsm->running, FSM_STATE_STOPPED);
    fsm_delete_pointer(fsm);
    fsm_delete_all_steps();
}

void test_fsm_rand_transition(void **state){
    struct fsm_pointer *fsm = fsm_create_pointer();
    struct fsm_step *step_0 = fsm_create_step(fsm_null_callback, NULL);
    struct fsm_step *step_1 = fsm_create_step(fsm_null_callback, NULL);
    struct fsm_step *step_2 = fsm_create_step(fsm_null_callback, NULL);

    fsm_connect_step(step_0, step_1, "GO_TO_1");
    fsm_connect_step(step_0, step_2, "GO_TO_2");

    char passed_by = 1; // Ensure we have test step 1 and 2 using primary numbers

    while(passed_by % 2 != 0 && passed_by % 3 != 0){
        fsm_start_pointer(fsm, step_0);

        char choice = rand() % 2 == 1;
        if (choice) {
            fsm_signal_pointer_of_event(fsm, fsm_generate_event("GO_TO_1", NULL));
        } else {
            fsm_signal_pointer_of_event(fsm, fsm_generate_event("GO_TO_2", NULL));
        }

        fsm_join_pointer(fsm);
        assert_ptr_not_equal(fsm->current_step, step_0);
        if (choice) {
            assert_ptr_equal(fsm->current_step, step_1);
            passed_by *= 2;
        } else {
            assert_ptr_equal(fsm->current_step, step_2);
            passed_by *= 3;
        }
    }

    fsm_delete_pointer(fsm);
    fsm_delete_all_steps();
}

void test_fsm_passing_value_by_step(void **state){
    struct fsm_pointer *fsm = fsm_create_pointer();
    int value = 5;
    struct fsm_step *step_0 = fsm_create_step(callback_set_int_from_step_to_42, (void *) &value);
    fsm_start_pointer(fsm, step_0);
    fsm_join_pointer(fsm);
    fsm_delete_pointer(fsm);
    fsm_delete_all_steps();
    assert_int_equal(value, 42);
}

void test_fsm_passing_value_by_event(void **state){
    struct fsm_pointer *fsm = fsm_create_pointer();
    int value = 42;
    struct fsm_step *step_0 = fsm_create_step(fsm_null_callback, NULL);
    struct fsm_step *step_1 = fsm_create_step(callback_assert_int_from_event_is_42, NULL);

    fsm_connect_step(step_0, step_1, "TEST_PASSING_VALUE");

    fsm_start_pointer(fsm, step_0);

    fsm_signal_pointer_of_event(fsm, fsm_generate_event("TEST_PASSING_VALUE", (void *) &value));

    fsm_join_pointer(fsm);
    assert_ptr_equal(fsm->current_step, step_1);
    fsm_delete_pointer(fsm);
    fsm_delete_all_steps();
}

void test_fsm_change_step_by_callback_return(void **state){
    struct fsm_pointer *fsm = fsm_create_pointer();
    struct fsm_step *step_0 = fsm_create_step(fsm_null_callback, NULL);
    struct fsm_step *step_1 = fsm_create_step(callback_return_step_from_event, NULL);
    struct fsm_step *step_2 = fsm_create_step(fsm_null_callback, NULL);
    struct fsm_step *step_3 = fsm_create_step(fsm_null_callback, NULL);


    fsm_connect_step(step_0, step_1, "TEST");
    fsm_connect_step(step_2, step_0, "RETURN");
    fsm_connect_step(step_3, step_0, "RETURN");

    fsm_start_pointer(fsm, step_0);

    fsm_signal_pointer_of_event(fsm, fsm_generate_event("TEST", (void *) step_2));
    pthread_mutex_lock(&fsm->mutex);
    while(fsm->current_step == step_1 || fsm->current_step == step_0){
        pthread_cond_wait(&fsm->cond_event, &fsm->mutex);
    }
    pthread_mutex_unlock(&fsm->mutex);
    assert_ptr_equal(step_2, fsm->current_step);
    fsm_signal_pointer_of_event(fsm, fsm_generate_event("RETURN", NULL));
    pthread_mutex_lock(&fsm->mutex);
    while(fsm->current_step != step_0){
        pthread_cond_wait(&fsm->cond_event, &fsm->mutex);
    }
    pthread_mutex_unlock(&fsm->mutex);
    fsm_signal_pointer_of_event(fsm, fsm_generate_event("TEST", (void *) step_3));
    pthread_mutex_lock(&fsm->mutex);
    while(fsm->current_step != step_3){
        pthread_cond_wait(&fsm->cond_event, &fsm->mutex);
    }
    pthread_mutex_unlock(&fsm->mutex);
    assert_ptr_equal(step_3, fsm->current_step);
    fsm_signal_pointer_of_event(fsm, fsm_generate_event("RETURN", NULL));

    fsm_join_pointer(fsm);
    fsm_delete_pointer(fsm);
    fsm_delete_all_steps();
}

void test_fsm_direct_transition(void **state){
    struct fsm_pointer *fsm = fsm_create_pointer();
    struct fsm_step *step_0 = fsm_create_step(fsm_null_callback, NULL);
    struct fsm_step *step_1 = fsm_create_step(fsm_null_callback, NULL);

    fsm_connect_step(step_0, step_1, _EVENT_DIRECT_TRANSITION_UID);

    fsm_start_pointer(fsm, step_0);
    assert_int_equal(fsm_wait_step_mstimeout(fsm, step_1, 1000), 0);

    fsm_join_pointer(fsm);
    fsm_delete_pointer(fsm);
    fsm_delete_all_steps();
}

void test_fsm_memory_persistence(void **state){
    struct fsm_pointer *fsm_base = fsm_create_pointer();
    struct fsm_pointer *fsm_dyn  = NULL;
    int value = 0;
    struct create_fsm data = {
            .pointer = &fsm_dyn,
            .data = &value,
    };
    struct fsm_step *step_0 = fsm_create_step(fsm_null_callback, NULL);
    struct fsm_step *step_1 = fsm_create_step(callback_create_fsm, (void *) &data);
    fsm_connect_step(step_0, step_1, _EVENT_DIRECT_TRANSITION_UID);
    fsm_start_pointer(fsm_base, step_0);
    fsm_wait_step_blocking(fsm_base, step_1);
    fsm_join_pointer(fsm_base);
    fsm_delete_pointer(fsm_base);
    struct fsm_step *step_0_dyn = fsm_dyn->current_step;
    fsm_signal_pointer_of_event(fsm_dyn, fsm_generate_event("GO", NULL));
    fsm_wait_leaving_step_blocking(fsm_dyn, step_0_dyn);
    fsm_join_pointer(fsm_dyn);
    assert_int_equal(value, 42);
    fsm_delete_pointer(fsm_dyn);
    fsm_delete_all_steps();
}

void test_fsm_ttl(void **state){
    struct fsm_pointer *fsm = fsm_create_pointer();
    struct fsm_step *step_0 = fsm_create_step(fsm_null_callback, NULL);
    struct fsm_step *step_1 = fsm_create_step(fsm_null_callback, NULL);
    struct fsm_step *step_2 = fsm_create_step(fsm_null_callback, NULL);

    fsm_connect_step(step_0, step_1, "GO");
    fsm_connect_step(step_1, step_2, "NEXT");

    fsm_start_pointer(fsm, step_0);

    fsm_signal_pointer_of_event(fsm, fsm_generate_event("NEXT", NULL));
    fsm_signal_pointer_of_event(fsm, fsm_generate_event("GO", NULL));

    assert_int_equal(fsm_wait_step_mstimeout(fsm, step_1, INT_MAX), 0);

    fsm_join_pointer(fsm);
    fsm_delete_pointer(fsm);

    struct fsm_config_pointer config = {
        .ttl_activated = true,
    };
    fsm = fsm_create_pointer_config(config);

    fsm_start_pointer(fsm, step_0);

    fsm_event *ttl_event = fsm_generate_event("NEXT", NULL);
    ttl_event->ttl = fsm_time_get_abs_fixed_time_from_us(INT_MAX);

    fsm_signal_pointer_of_event(fsm, ttl_event);
    fsm_signal_pointer_of_event(fsm, fsm_generate_event("GO", NULL));

    fsm_wait_step_blocking(fsm, step_2);

    fsm_join_pointer(fsm);
    fsm_delete_pointer(fsm);

    fsm_delete_all_steps();
}

void test_fsm_out_action(void **state){
    struct fsm_pointer *fsm = fsm_create_pointer();
    int value = 0;
    struct fsm_step *step_0 = fsm_create_step(fsm_null_callback, (void *) &value);
    step_0->out_fnct = callback_set_int_from_step_to_42;
    struct fsm_step *step_1 = fsm_create_step(fsm_null_callback, NULL);
    struct fsm_step *step_2 = fsm_create_step(fsm_null_callback, NULL);
    fsm_connect_step(step_0, step_1, "STEP1");
    fsm_connect_step(step_0, step_2, "STEP2");
    fsm_connect_step(step_1, step_0, "STEP0");
    fsm_connect_step(step_2, step_0, "STEP0");
    fsm_start_pointer(fsm, step_0);
    assert_int_equal(value, 0);
    fsm_signal_pointer_of_event(fsm, fsm_generate_event("STEP1", NULL));
    assert_int_equal(fsm_wait_step_mstimeout(fsm,step_1,500), 0);
    assert_int_equal(value, 42);
    fsm_signal_pointer_of_event(fsm, fsm_generate_event("STEP0", NULL));
    value = 1;
    assert_int_equal(value, 1);
    fsm_signal_pointer_of_event(fsm, fsm_generate_event("STEP2", NULL));
    assert_int_equal(fsm_wait_step_mstimeout(fsm,step_2,500), 0);
    assert_int_equal(value, 42);
    fsm_join_pointer(fsm);
    fsm_delete_pointer(fsm);
    fsm_delete_all_steps();
}

void test_fsm_simple_timeout(void **state){
    struct fsm_pointer *fsm = fsm_create_pointer();
    struct fsm_step *step_0 = fsm_create_step(fsm_null_callback, NULL);
    struct fsm_step *step_1 = fsm_create_step(fsm_null_callback, NULL);
    struct fsm_step *step_2 = fsm_create_step(fsm_null_callback, NULL);
    fsm_connect_step(step_0, step_1, "STEP1");
    fsm_connect_step(step_0, step_2, _EVENT_TIMEOUT_UID);
    fsm_connect_step(step_1, step_0, "STEP0");
    fsm_connect_step(step_2, step_0, "STEP0");
    fsm_set_timeout_to_step(step_0, 100000); // Wait 100ms
    fsm_start_pointer(fsm, step_0);

    assert_int_not_equal(fsm_wait_step_mstimeout(fsm, step_2, 5000), ETIMEDOUT);
    fsm_signal_pointer_of_event(fsm, fsm_generate_event("STEP0", NULL));
    assert_int_not_equal(fsm_wait_step_mstimeout(fsm, step_2, 5000), ETIMEDOUT);
    fsm_signal_pointer_of_event(fsm, fsm_generate_event("STEP0", NULL));
    fsm_signal_pointer_of_event(fsm, fsm_generate_event("STEP1", NULL));
    assert_int_not_equal(fsm_wait_step_mstimeout(fsm, step_1, 5000), ETIMEDOUT);

    fsm_join_pointer(fsm);
    fsm_delete_pointer(fsm);
    fsm_delete_all_steps();
}

void test_fsm_simple_conditional_transition(void **state){
    struct fsm_pointer *fsm = fsm_create_pointer();
    int signal = 0;
    int value = 0;
    struct fsm_step *step_0 = fsm_create_step(callback_set_int_from_step_to_42, (void *)&value);
    struct fsm_step *step_1 = fsm_create_step(fsm_null_callback, NULL);
    struct fsm_step *step_2 = fsm_create_step(fsm_null_callback, NULL);
    callback_pointer_step_1 = step_1;
    callback_pointer_step_2 = step_2;

    fsm_add_conditional_transition_to_step(step_0, "GO", callback_condtrans_step1_or_2);
    fsm_connect_step(step_1, step_0, "STEP0");
    fsm_connect_step(step_2, step_0, "STEP0");
    fsm_start_pointer(fsm, step_0);

    assert_int_not_equal(fsm_wait_step_mstimeout(fsm, step_0, AVG_WAIT_STEP_TIMEOUT_MS), ETIMEDOUT);
    signal = 1;
    fsm_signal_pointer_of_event(fsm, fsm_generate_event("GO", (void *)&signal));
    assert_int_not_equal(fsm_wait_step_mstimeout(fsm, step_1, AVG_WAIT_STEP_TIMEOUT_MS), ETIMEDOUT);

    fsm_signal_pointer_of_event(fsm, fsm_generate_event("STEP0", NULL));
    assert_int_not_equal(fsm_wait_step_mstimeout(fsm, step_0, AVG_WAIT_STEP_TIMEOUT_MS), ETIMEDOUT);
    signal = 2;
    fsm_signal_pointer_of_event(fsm, fsm_generate_event("GO", (void *)&signal));
    assert_int_not_equal(fsm_wait_step_mstimeout(fsm, step_2, AVG_WAIT_STEP_TIMEOUT_MS), ETIMEDOUT);

    fsm_signal_pointer_of_event(fsm, fsm_generate_event("STEP0", NULL));
    assert_int_not_equal(fsm_wait_step_mstimeout(fsm, step_0, AVG_WAIT_STEP_TIMEOUT_MS), ETIMEDOUT);
    signal = 0;
    value = 0;
    fsm_signal_pointer_of_event(fsm, fsm_generate_event("GO", (void *)&signal));
    assert_int_equal(value, 0);


    fsm_join_pointer(fsm);
    fsm_delete_pointer(fsm);
    fsm_delete_all_steps();
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
            cmocka_unit_test(test_fsm_memory_persistence),
            cmocka_unit_test(test_fsm_ttl),
            cmocka_unit_test(test_fsm_out_action),
            cmocka_unit_test(test_fsm_simple_timeout),
            cmocka_unit_test(test_fsm_simple_conditional_transition),
    };

    int rc = cmocka_run_group_tests(tests, NULL, NULL);
    return rc;
}