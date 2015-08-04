//#include "fsm.h"
//#include<unistd.h>
#include <stdio.h>
//#include <stdlib.h>
//#include <pthread.h>
#include <stdlib.h>
#include "pthread.h"
//#include <stdio.h>

#include "fsm.h"
#include "fsm_event_queue.h"
#include "unistd.h"
#include "debug.h"


void test_fsm(){
    //printf("Start test_fsm \n");
    //struct test a = {0,1};
    //char str[] = {'h', 'e', 'l', 'l', 'o', 0};
    //char str[] = "Single";
    //struct mxd_event sure_event = {1, 0.5, c};
    //struct test_in b = {2,3};
    int i = 0;
    int j = 10;
    int f = i + j;
   /* struct step step2 = {&callback, '2', NULL};
    struct event key_event = {11, '0'};
    const struct transition event_trans = {key_event.x,&step2};
    struct step step1 = {&callback, '1', NULL};
    const struct transition init_trans = {START_EVENT.x,&step1};
    struct step step0 = {&callback, 0, &init_trans};
    struct event event_init = {0,'b'};*/
 /*   struct pointer fsm = {
            .input = NULL,
            .thread = NULL,
            .cond_new_input = PTHREAD_COND_INITIALIZER,
            .mutex = PTHREAD_MUTEX_INITIALIZER,
    };*/
   // start_fsm(&fsm, step0);
/*
    sleep(1);

  //  pointer_event_signal(&key_event, &fsm);

    sleep(1);

  //  pointer_event_signal(&_END_POINTER, &fsm);

    sleep(1);

    sleep(50);

 //   pthread_join(fsm.thread, NULL);
    printf("End test_fsm \n"); */
}



void old_test_new_fsm(){
    struct fsm_step step_0 = {
            .fnct = &callback,
            .args = NULL,
            .transition = TRANS_ENDPOINT,
    }, step_1 = {
            .fnct = &callback,
            .args = NULL,
            .transition = TRANS_ENDPOINT,
    };
    struct fsm_trans trans_0_1 = {
            .event_uid = START_EVENT.uid,
            .next_step = &step_1,
    };
    step_0.transition = trans_0_1;
    struct fsm_context context_0_1 = {
            .event = START_EVENT,
            .fnct_args = step_1.args,
    };
    step_0.transition.next_step->fnct(&context_0_1);

}

void old2_test_new_fsm(){
    struct fsm_step step_0 = create_step(&callback, NULL);
    struct fsm_step step_1 = create_step(&callback, NULL);
    connect_step(&step_0, &step_1, START_EVENT.uid);
    struct fsm_context context_0_1 = {
            .event = START_EVENT,
            .fnct_args = step_1.args,
    };
    step_0.transition.next_step->fnct(&context_0_1);
}

void test_new_fsm(){
    struct fsm_step step_0 = create_step(&callback, NULL);
    struct fsm_step step_1 = create_step(&callback, NULL);
    connect_step(&step_0, &step_1, START_EVENT.uid);

    struct fsm_pointer *fsm = create_pointer(step_0);

    start_pointer(fsm);
    pthread_join(fsm->thread, NULL);
    free(fsm);

}

void test_queue_event(){
    struct fsm_queue queue = create_fsm_queue();
    printf("Event start addr %u, uid : %d  \n", &START_EVENT, START_EVENT.uid);
    printf("Event _END_POINTER addr %u, uid : %d \n", &_END_POINTER, _END_POINTER.uid);
    printf("Event _NONE_EVENT addr %u, uid : %d \n", &_NONE_EVENT, _NONE_EVENT.uid);

    struct fsm_event * pSTART = push_back_fsm_event_queue(&queue, &START_EVENT);
    struct fsm_event * pEND = push_back_fsm_event_queue(&queue, &_END_POINTER);
    struct fsm_event * pNONE = push_back_fsm_event_queue(&queue, &_NONE_EVENT);
    struct fsm_event *event;
    event = pop_front_fsm_event_queue(&queue);
    printf("Event from queue addr %u/%u, uid : %d \n", event, pSTART, event->uid);
    free(event);
    event = pop_front_fsm_event_queue(&queue);
    printf("Event from queue addr %u/%u, uid : %d \n", event, pEND, event->uid);
    free(event);
    event = pop_front_fsm_event_queue(&queue);
    printf("Event from queue addr %u/%u, uid : %d \n", event, pNONE, event->uid);
    free(event);

}

void test_fsm_loop(){
    for (int i = 0; i<10000; i++) {
        struct fsm_step step_0 = create_step(&callback, NULL);
        struct fsm_step step_1 = create_step(&callback, NULL);
        connect_step(&step_0, &step_1, START_EVENT.uid);

        struct fsm_pointer *fsm = create_pointer(step_0);

        start_pointer(fsm);
        //sleep(2);
        //log_info("First event");
        signal_fsm_pointer_of_event(fsm, &START_EVENT);
        //sleep(5);
        //log_info("Second event");
        signal_fsm_pointer_of_event(fsm, &_END_POINTER);

        pthread_join(fsm->thread, NULL);
        destroy_pointer(fsm);
    }

}



int main(){

    //test_fsm();

    //test_new_fsm();

    //test_queue_event();

    test_fsm_loop();

    sleep(1);
    int i = 0;
    i = i + 1;

    return 0;
}