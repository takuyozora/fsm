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

#include "fsm_queue.h"
#include "fsm_debug.h"

void test_queue_push_pop_order(void **state){
    struct fsm_queue queue = create_fsm_queue();
    int valueRand1 = rand();
    int valueRand2 = rand();
    int valueRand3 = rand();
    while (valueRand1 == valueRand2){
        valueRand2 = rand();        // assert theses two values are different
    }
    while (valueRand3 == valueRand1 || valueRand3 == valueRand2){
        valueRand3 = rand();
    }
    int * pValue1 = fsm_queue_push_back(&queue, (void *) &valueRand1, sizeof(int));
    int * pValue2 = fsm_queue_push_back(&queue, (void *) &valueRand2, sizeof(int));
    int * pValue3 = fsm_queue_push_back(&queue, (void *) &valueRand3, sizeof(int));

    int * pValuePop1 = fsm_queue_pop_front(&queue);
    assert_int_equal(*pValue1, *pValuePop1);
    assert_ptr_equal(pValue1, pValuePop1);

    int * pValuePop2 = fsm_queue_pop_front(&queue);
    assert_int_equal(*pValue2, *pValuePop2);
    assert_ptr_equal(pValue2, pValuePop2);

    int * pValuePop3 = fsm_queue_pop_front(&queue);
    assert_int_equal(*pValue3, *pValuePop3);
    assert_ptr_equal(pValue3, pValuePop3);

    assert_int_not_equal(*pValuePop1, *pValuePop2);
    assert_int_not_equal(*pValuePop1, *pValuePop3);
    assert_int_not_equal(*pValuePop2, *pValuePop3);

    // Must free all data coming up from a queue
    free(pValue1);
    free(pValue2);
    free(pValue3);

    fsm_queue_cleanup(&queue);
}


void * _test_queue_signal_producer(void * _queue){
    struct fsm_queue *queue = _queue;
    int value = 42;
    fsm_queue_push_back(queue, (void *) &value, sizeof(int));
    pthread_cond_signal(&queue->cond);
    return NULL;
}

void test_queue_signal(void **state){
    struct fsm_queue queue = create_fsm_queue();
    int wait = 0;
    pthread_t thread;
    for(int i=0; i<100; i++) {
        pthread_create(&thread, NULL, _test_queue_signal_producer, (void *) &queue);
        pthread_mutex_lock(&queue.mutex);
        while (queue.first == NULL) {
            pthread_cond_wait(&queue.cond, &queue.mutex);
            wait++;
        }
        pthread_mutex_unlock(&queue.mutex);
        int *pValue = fsm_queue_pop_front(&queue);
        assert_int_equal(42, *pValue);
        free(pValue);
        fsm_queue_cleanup(&queue);
        while(pthread_join(thread, NULL) != 0){
            debug("Error on loop %d joining thread ", i);
        }
    }
    assert_int_not_equal(wait, 0); // Just to be sure than at least one turn test the wait way

}

int main(void)
{
    srand ((unsigned int) time(NULL));
    const struct CMUnitTest tests[2] = {
            cmocka_unit_test(test_queue_push_pop_order),
            cmocka_unit_test(test_queue_signal)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
