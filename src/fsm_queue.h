//
// Created by olivier on 04/08/15.
//

#ifndef FSM_QUEUE_H
#define FSM_QUEUE_H

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
struct fsm_queue * create_fsm_queue_pointer();
void *fsm_queue_push_back(struct fsm_queue *queue, void *_value, const unsigned short size);
void *fsm_queue_push_back_more(struct fsm_queue *queue, void *_value,
                               const unsigned short size, unsigned short copy);
void *fsm_queue_pop_front(struct fsm_queue *queue);
/*! Search an elem into the queue and get it
 *      @param queue Pointer to the fsm_queue in which search
 *      @param elem Generic void pointer to the elem
 *
 *  @retval NULL if the given elem isn't found
 *  @retval elem otherwise
 *
 *  @note the elem is removed from the queue if is found
 */
void *fsm_queue_get_elem(struct fsm_queue *queue, void *elem);
unsigned short cleanup_fsm_queue(struct fsm_queue *queue);
unsigned short destory_fsm_queue_pointer(struct fsm_queue *queue);


#endif //FSM_QUEUE_H
