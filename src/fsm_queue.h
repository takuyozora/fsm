//
// Created by olivier on 04/08/15.
//

/*!
 * \file fsm_queue.h
 * \brief Thread safe queue used by fsm.h
 * \author Olivier Radisson <olivier.radisson _at_ insa-lyon.fr>
 * \version 0.1
 */

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

/*! Create a fsm_queue and return it
 */
struct fsm_queue create_fsm_queue ();

/* Create a fsm_queue in heap memory and return a pointer to it
 *
 * @return pointer to fsm_queue
 *
 * @note It uses \c malloc for the fsm_queue allocation : you should free it at the end of his usage
 * */
struct fsm_queue * create_fsm_queue_pointer();

/*! Copy an element into the heap and store it at the end of the queue
 *      @param queue Pointer to the fsm_queue
 *      @param _value Generic void pointer to the element to store
 *      @param size Size in bytes (as return by \a sizeof) of the pointed variable
 *
 *  @return Generic void pointer to the memory allocated
 *
 *  This function copy the element you give into the heap and store a pointer of it into the queue. It returns this pointer to you.
 *
 *  @note It uses \c malloc in order to copy the value you give, you must be carefull about memory leaks.
 *
 *  @warning Do not free the returned pointer if you want to use fsm_queue_cleanup(fsm_queue *) function.
 *
 *  @see fsm_queue_push_back_more(struct fsm_queue *, void *, const unsigned short, unsigned short copy)
 *  @see  fsm_queue_cleanup(fsm_queue *)
 *  */
void *fsm_queue_push_back(struct fsm_queue *queue, void *_value, const unsigned short size);

/*! Push an element at the end of the queue
 *      @param queue Pointer to the fsm_queue
 *      @param _value Generic void pointer to the element to store
 *      @param size Size in bytes (as return by \a sizeof) of the pointed variable
 *      @param copy Set to 1 if you want to copy the element into the heap, just store the given generic void pointer otherwise.
 *
 *  @retval Generic void pointer to the memory allocated if `copy == 1`.
 *  @retval Given _value pointer otherwise.
 *
 *  This function can copy the element you give into the heap and store a pointer of it into the queue. It returns this pointer to you.
 *
 *  @note It (can) uses \c malloc in order to copy the value you give, you must be carefull about memory leaks.
 *
 *  @warning Do not free the returned pointer if you want to use fsm_queue_cleanup(fsm_queue *) function.
 *
 *  @see fsm_queue_push_back(struct fsm_queue *, void *, const unsigned short)
 *  @see fsm_queue_cleanup(fsm_queue *)
 *  */
void *fsm_queue_push_back_more(struct fsm_queue *queue, void *_value,
                               const unsigned short size, unsigned short copy);

void *fsm_queue_push_top_more(struct fsm_queue *queue, void *_value,
                               const unsigned short size, unsigned short copy);

void *fsm_queue_push_top(struct fsm_queue *queue, void *_value, const unsigned short size);

/*! Pop an element from the tail of the queue and return a generic pointer to it
 *      @param Pointer to the fsm_queue
 *
 * @retval Generic void pointer to the popped element
 * @retval NULL if the queue is empty
 *
 * */
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

/*! Pop all elements into the queue in order to clean it
 *      @param queue Pointer to the fsm_queue
 *
 *  @warning It frees all values stored into the queue so if they've been freed earlier it will cause an segmentation fault.
 *
 *  @see fsm_queue_push_back_more(struct fsm_queue *, void *, const unsigned short, unsigned short)
 *  @see fsm_queue_push_back(struct fsm_queue *, void *, const unsigned short)
 *  */
void fsm_queue_cleanup(struct fsm_queue *queue);

/*! Cleanup the given fsm_queue and free it after
 *      @param queue Pointer to the fsm_queue to delete
 *
 *  @warning It frees all values stored into the queue so if they've been freed earlier it will cause an segmentation fault.
 *
 *  @see fsm_queue_cleanup(struct fsm_queue *)
 *  */
void fsm_queue_delete_queue_pointer(struct fsm_queue *queue);


#endif //FSM_QUEUE_H
