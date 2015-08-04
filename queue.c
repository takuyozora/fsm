//
// Created by olivier on 04/08/15.
//

#include <stdlib.h>
#include <string.h>
#include "queue.h"


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

void push_back_fsm_queue(struct fsm_queue *queue, const void *_value, const unsigned short size) {
    struct fsm_queue_elem elem;
    elem.value = malloc(sizeof(size));
    memcpy(elem.value, _value, size);
    elem.next = NULL;
    pthread_mutex_lock(&queue->mutex);
    elem.prev = queue->last;
    elem.prev->next = &elem;
    queue->last = &elem;
    pthread_mutex_unlock(&queue->mutex);
    pthread_cond_signal(&queue->cond);
}

void *pop_front_fsm_queue(struct fsm_queue *queue) {
    if (queue->first == NULL){
        return NULL;
    }
    pthread_mutex_lock(&queue->mutex);
    void * value = queue->first->value;
    queue->first = queue->first->next;
    if(queue->first != NULL) {
        queue->first->prev = NULL;
    }
    pthread_mutex_unlock(&queue->mutex);
    return value;
}
