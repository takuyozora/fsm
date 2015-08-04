//
// Created by olivier on 04/08/15.
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "debug.h"
#include "fsm_queue.h"


struct fsm_queue create_fsm_queue() {
    struct fsm_queue queue = {
            .first = NULL,
            .last = NULL,
            .mutex = NULL,
            .cond = NULL,
    };
    check(pthread_mutex_init(&queue.mutex, NULL) == 0, "ERROR DURING MUTEX INIT");
    check(pthread_cond_init(&queue.cond, NULL) == 0, "ERROR DURING CONDITION INIT");
    return queue;
    error:
    exit(1);
}

void * push_back_fsm_queue_more(
        struct fsm_queue *queue, void *_value, const unsigned short size, unsigned short copy) {
    struct fsm_queue_elem * elem = malloc(sizeof(struct fsm_queue_elem));
    if (copy) {
        elem->value = malloc(size);
        memcpy(elem->value, _value, size);
    }else{
        elem->value = _value;
    }
    elem->next = NULL;
    pthread_mutex_lock(&queue->mutex);
    elem->prev = queue->last;
    if (elem->prev != NULL) {
        elem->prev->next = elem;
    }else{
        queue->first = elem;
    }
    queue->last = elem;
    pthread_cond_broadcast(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
    return elem->value;
}

void * push_back_fsm_queue(
        struct fsm_queue *queue, void *_value, const unsigned short size) {
    return push_back_fsm_queue_more(queue, _value, size, 1);
}

void *pop_front_fsm_queue(struct fsm_queue *queue) {
    pthread_mutex_lock(&queue->mutex);
    if (queue->first == NULL){
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }
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

unsigned short cleanup_fsm_queue(struct fsm_queue *queue) {
    while(queue->first != NULL){
        free(pop_front_fsm_queue(queue));
    }
    return 0;
}

struct fsm_queue *create_fsm_queue_pointer() {
    struct fsm_queue _q = create_fsm_queue();
    struct fsm_queue * queue = malloc(sizeof(struct fsm_queue));
    memcpy(queue, &_q, sizeof(struct fsm_queue));
    return queue;
}

unsigned short destory_fsm_queue_pointer(struct fsm_queue *queue) {
    cleanup_fsm_queue(queue);
    free(queue);
    return 0;
}