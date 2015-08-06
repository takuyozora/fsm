//
// Created by olivier on 04/08/15.
//

#include <unistd.h>
#include "fsm_event_queue.h"
#include "debug.h"

struct fsm_event *pop_front_fsm_event_queue(struct fsm_queue *queue){
    return (struct fsm_event *) fsm_queue_pop_front(queue);
}

struct fsm_event * push_back_fsm_event_queue(struct fsm_queue *queue, struct fsm_event *event) {
    return (struct fsm_event *) fsm_queue_push_back_more(queue, (void *) event, sizeof(event),
                                                         0);
}

struct fsm_event *get_event_or_wait(struct fsm_queue *queue) {
    pthread_mutex_lock(&queue->mutex);
    while(queue->first == NULL) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }
    pthread_mutex_unlock(&queue->mutex);
    return pop_front_fsm_event_queue(queue);
}