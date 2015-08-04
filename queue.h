//
// Created by olivier on 04/08/15.
//

#ifndef FSM_1_QUEUE_H
#define FSM_1_QUEUE_H

#include "pthread.h"

struct fsm_queue_elem {
    struct fsm_queue_elem * next;
    struct fsm_queue_elem * prev;
    void * value;
};

struct fsm_queue {
    struct fsm_queue_elem * first;
    struct fsm_queue_elem * last;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

struct fsm_queue create_fsm_queue ();
void push_back_fsm_queue(struct fsm_queue *queue, const void *_value, const unsigned short size);
void * pop_front_fsm_queue (struct fsm_queue *queue);

#endif //FSM_1_QUEUE_H
