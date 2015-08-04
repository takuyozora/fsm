//
// Created by olivier on 04/08/15.
//

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "fsm_queue.h"
#include "pthread.h"

void test_queue_push_pop_order(){
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
    int * pValue1 = push_back_fsm_queue(&queue, (void *)&valueRand1, sizeof(int));
    int * pValue2 = push_back_fsm_queue(&queue, (void *)&valueRand2, sizeof(int));
    int * pValue3 = push_back_fsm_queue(&queue, (void *)&valueRand3, sizeof(int));

    int * pValuePop1 = pop_front_fsm_queue(&queue);
    assert_int_equal(*pValue1, *pValuePop1);
    assert_ptr_equal(pValue1, pValuePop1);

    int * pValuePop2 = pop_front_fsm_queue(&queue);
    assert_int_equal(*pValue2, *pValuePop2);
    assert_ptr_equal(pValue2, pValuePop2);

    int * pValuePop3 = pop_front_fsm_queue(&queue);
    assert_int_equal(*pValue3, *pValuePop3);
    assert_ptr_equal(pValue3, pValuePop3);

    assert_int_not_equal(*pValuePop1, *pValuePop2);
    assert_int_not_equal(*pValuePop1, *pValuePop3);
    assert_int_not_equal(*pValuePop2, *pValuePop3);

    cleanup_fsm_queue(&queue);
}


void * _test_queue_signal_producer(void * _queue){
    struct fsm_queue *queue = _queue;
    int value = 42;
    usleep(500);
    push_back_fsm_queue(queue, (void *)&value, sizeof(int));
    pthread_cond_signal(&queue->cond);
    return NULL;
}

void test_queue_signal(){
    struct fsm_queue queue = create_fsm_queue();
    pthread_t thread;
    int wait = 0;
    for(int i=0; i<100; i++) {
        pthread_create(&thread, NULL, &_test_queue_signal_producer, (void *) &queue);
        while (queue.first == NULL) {
            pthread_cond_wait(&queue.cond, &queue.mutex);
            wait++;
        }
        pthread_mutex_unlock(&queue.mutex);
        int *pValue = pop_front_fsm_queue(&queue);
        assert_int_equal(42, *pValue);
        cleanup_fsm_queue(&queue);
        pthread_join(thread, NULL);
    }
    assert_int_not_equal(wait, 0); // Just to be sure than at least one turn test the wait way
}

int main(void)
{
    srand ((unsigned int) time(NULL));
    const UnitTest tests[] = {
            unit_test(test_queue_push_pop_order),
            unit_test(test_queue_signal),
    };

    return run_tests(tests);
}
