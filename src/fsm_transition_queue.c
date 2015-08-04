//
// Created by olivier on 04/08/15.
//

#include "fsm_transition_queue.h"

struct fsm_transition *push_back_fsm_transition_queue(struct fsm_queue *queue, struct fsm_transition *transition) {
    return (struct fsm_transition *)push_back_fsm_queue(queue, (void *)transition, sizeof(*transition));
}

struct fsm_transition *get_reachable_condition(struct fsm_queue *queue, struct fsm_event *event) {
    pthread_mutex_lock(&queue->mutex);
    struct fsm_queue_elem *cursor = queue->first;
    do{
        if(((struct fsm_transition *)(cursor->value))->event_uid == event->uid){
            pthread_mutex_unlock(&queue->mutex);
            return ((struct fsm_transition *)(cursor->value));
        }
        cursor = cursor->next;
    }while (cursor != NULL);
    pthread_mutex_unlock(&queue->mutex);
    return NULL;
}
