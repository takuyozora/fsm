//
// Created by olivier on 04/08/15.
//

#include <stdlib.h>
#include <string.h>
#include "fsm_queue.h"


struct fsm_queue create_fsm_queue() {
    struct fsm_queue queue = {
            .first = NULL,
            .last = NULL,
            .mutex = NULL,
            .cond = NULL,
    };
    pthread_mutex_init(&queue.mutex, NULL);
    pthread_cond_init(&queue.cond, NULL);
    return queue;
}

void * push_back_fsm_queue(struct fsm_queue *queue, const void *_value, const unsigned short size) {
    struct fsm_queue_elem * elem = malloc(sizeof(struct fsm_queue_elem));
    elem->value = malloc(sizeof(size));
    memcpy(elem->value, _value, size);
    elem->next = NULL;
    pthread_mutex_lock(&queue->mutex);
    elem->prev = queue->last;
    if (elem->prev != NULL) {
        elem->prev->next = elem;
    }else{
        queue->first = elem;
    }
    queue->last = elem;
    pthread_mutex_unlock(&queue->mutex);
    pthread_cond_signal(&queue->cond);
    return elem->value;
}

void *pop_front_fsm_queue(struct fsm_queue *queue) {
    if (queue->first == NULL){
        return NULL;
    }
    pthread_mutex_lock(&queue->mutex);
    void * value = queue->first->value;
    struct fsm_queue_elem *fsm_elem_to_free = queue->first;
    queue->first = queue->first->next;
    free(fsm_elem_to_free);
    if(queue->first != NULL) {
        queue->first->prev = NULL;
    }else{
        queue->last = NULL;
    }
    pthread_mutex_unlock(&queue->mutex);
    return value;
}
