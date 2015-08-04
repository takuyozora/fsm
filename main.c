//#include "fsm.h"
//#include<unistd.h>
#include <stdio.h>
//#include <stdlib.h>
//#include <pthread.h>
#include <stdlib.h>
#include "pthread.h"
//#include <stdio.h>

#include "src/fsm.h"
#include "src/fsm_event_queue.h"
#include "unistd.h"
#include "src/debug.h"


//
//void old_test_new_fsm(){
//    struct fsm_step step_0 = {
//            .fnct = &callback,
//            .args = NULL,
//            .transition = TRANS_ENDPOINT,
//    }, step_1 = {
//            .fnct = &callback,
//            .args = NULL,
//            .transition = TRANS_ENDPOINT,
//    };
//    struct fsm_transition trans_0_1 = {
//            .event_uid = START_EVENT.uid,
//            .next_step = &step_1,
//    };
//    step_0.transition = trans_0_1;
//    struct fsm_context context_0_1 = {
//            .event = START_EVENT,
//            .fnct_args = step_1.args,
//    };
//    step_0.transition.next_step->fnct(&context_0_1);
//
//}
//
//void old2_test_new_fsm(){
//    struct fsm_step step_0 = create_step(&callback, NULL);
//    struct fsm_step step_1 = create_step(&callback, NULL);
//    connect_step(&step_0, &step_1, START_EVENT.uid);
//    struct fsm_context context_0_1 = {
//            .event = START_EVENT,
//            .fnct_args = step_1.args,
//    };
//    step_0-transition.next_step->fnct(&context_0_1);
//}

void test_new_fsm(){
    struct fsm_step *step_0 = create_step(&callback, NULL);
    struct fsm_step *step_1 = create_step(&callback, NULL);
    connect_step(step_0, step_1, START_EVENT.uid);

    struct fsm_pointer *fsm = create_pointer();

    start_pointer(fsm, step_0);
    pthread_join(fsm->thread, NULL);
    free(fsm);
    free(step_0);

}

void test_queue_event(){
    struct fsm_queue queue = create_fsm_queue();
    printf("Event start addr %u, uid : %d  \n", &START_EVENT, START_EVENT.uid);
    printf("Event _END_POINTER addr %u, uid : %d \n", &_END_POINTER, _END_POINTER.uid);
    printf("Event _NONE_EVENT addr %u, uid : %d \n", &_NONE_EVENT, _NONE_EVENT.uid);

    struct fsm_event * pSTART = push_back_fsm_event_queue(&queue, START_EVENT);
    struct fsm_event * pEND = push_back_fsm_event_queue(&queue, _END_POINTER);
    struct fsm_event * pNONE = push_back_fsm_event_queue(&queue, _NONE_EVENT);
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
        int arg_step_1 = 11, arg_step_0 = 10;
        struct fsm_step *step_0 = create_step(&callback, (void *)&arg_step_0);
        struct fsm_step *step_1 = create_step(&callback, (void *)&arg_step_1);
        connect_step(step_0, step_1, START_EVENT.uid);

        struct fsm_pointer *fsm = create_pointer();

        start_pointer(fsm, step_0);
        //sleep(2);
        //log_info("First event");
        signal_fsm_pointer_of_event(fsm, START_EVENT);
        //sleep(5);
        //log_info("Second event");
        signal_fsm_pointer_of_event(fsm, _END_POINTER);

        pthread_join(fsm->thread, NULL);
        destroy_pointer(fsm);
    }

}


void test_fsm_trans(){
    int arg_step_1 = 11, arg_step_0 = 10, arg_step_2=12;
    struct fsm_step *step_0 = create_step(&callback, (void *)&arg_step_0);
    struct fsm_step *step_1 = create_step(&callback, (void *)&arg_step_1);
    struct fsm_step *step_2 = create_step(&callback, (void *)&arg_step_2);

    /*struct fsm_event go_to_1 = {
            .uid = 101,
            .args = NULL,
    };
    struct fsm_event go_to_2 = {
            .uid = 102,
            .args = NULL,
    };*/

    short go_to_1_uid = 101, go_to_2_uid = 102;

    connect_step(step_0, step_1, go_to_1_uid);
    connect_step(step_0, step_2, go_to_2_uid);

    struct fsm_pointer *fsm = create_pointer();

    start_pointer(fsm, step_0);
    if(rand() % 2 == 1){
        debug("GO TO 1");
        signal_fsm_pointer_of_event(fsm, generate_event(go_to_1_uid, NULL));
    }else{
        debug("GO TO 2");
        signal_fsm_pointer_of_event(fsm, generate_event(go_to_2_uid, NULL));
    }

    signal_fsm_pointer_of_event(fsm, _END_POINTER);

    pthread_join(fsm->thread, NULL);
    destroy_pointer(fsm);

}



int main(){

    srand ((unsigned int) time(NULL));

    //test_fsm();

    //test_new_fsm();

    //test_queue_event();

    // test_fsm_loop();

    test_fsm_trans();



    sleep(1);
    int i = 0;
    i = i + 1;

    return 0;
}