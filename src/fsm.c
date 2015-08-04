//
// Created by olivier on 03/08/15.
//

#include <stdlib.h>
#include "fsm.h"
#include "fsm_queue.h"
#include "fsm_event_queue.h"
#include "fsm_transition_queue.h"
#include "stdio.h"
#include "debug.h"


struct fsm_transition TRANS_ENDPOINT = {0, NULL};
struct fsm_event START_EVENT = {1, NULL};
struct fsm_event _END_POINTER = {-1, NULL};
struct fsm_event _NONE_EVENT = {-2, NULL};
struct fsm_queue *_all_steps_created = NULL;


/*! Just a example callback function
 */
void * callback(const struct fsm_context *context) {
    log_info("Callback : event uid : %d, args : %d \n", context->event.uid, *(int *)context->fnct_args);
    return NULL;
}

/*! Simplify step creation.
 *
 *  Create a step from a function and a void pointer as argument for it.
 *  This function initialize transition to TRANS_ENDPOINT, a null transition
 *
 */
struct fsm_step * create_step(void *(*fnct)(const struct fsm_context *), void *args) {
    if (_all_steps_created == NULL){
        _all_steps_created = create_fsm_queue_pointer();
    }
    struct fsm_step *step = malloc(sizeof(struct fsm_step));
    step->fnct = fnct;
    step->args = args;
    step->transitions = create_fsm_queue_pointer();
    return step;
}

/*! Connect two step with an event by creating a transition
 *
 * Create a transition for the given event which refer to the \a to step and
 * attach it to the \a from step
 *
 */
void connect_step(struct fsm_step *from, struct fsm_step *to, short event_uid) {
    struct fsm_transition transition = {
            .event_uid = event_uid,
            .next_step = to,
    };
    push_back_fsm_transition_queue(from->transitions, &transition);
}

/*! Create a pointer wich is the main part of a FSM
 *
 * Create a pointer. Do not start it and just init other thread vars
 *
 */
struct fsm_pointer *create_pointer()
{
    struct fsm_pointer *pointer = malloc(sizeof(struct fsm_pointer));
    pointer->thread = 0;
    pthread_mutex_init(&pointer->mutex, NULL);
    pthread_cond_init(&pointer->cond_event, NULL);
    pointer->input_event = create_fsm_queue();
    pointer->current_step = NULL;
    pointer->running = 0;
    return pointer;
}

/*! Start the given pointer at the given step in a thread
 *
 * Check if the pointer isn't already running, if not start it
 * in a new thread on the given step
 *
 */
void start_pointer(struct fsm_pointer *pointer, struct fsm_step *init_step) {
    pthread_mutex_lock(&pointer->mutex);
    if ( pointer->running != 0 ) {
        log_err("CRITICAL : A pointer can't be running twice a time");
        return;
    }
    pointer->current_step = init_step;
    pthread_create(&(pointer->thread), NULL, &pointer_loop, (void *)pointer);
    pointer->running = 1;
    pthread_mutex_unlock(&pointer->mutex);
}

/*! Main loop for the pointer thread, run step and wait for event
 *
 * The main loop of a pointer thread execute a step and wait for a transition
 * when this one ended.
 *
 * This function shouldn't be called from an other place that start_pointer function
 *
 * NOT FULLY IMPLEMENTED
 */
void *pointer_loop(void * _pointer) {
    struct fsm_pointer * pointer = _pointer;
    struct fsm_step * ret_step;
    struct fsm_event * new_event = NULL;
    struct fsm_transition * reachable_transition = NULL;
    ret_step = start_step(pointer, pointer->current_step, &START_EVENT);
    while (1){
        if(ret_step != NULL){
            ret_step = start_step(pointer, ret_step, &_NONE_EVENT);
            continue;
        }
        new_event = get_event_or_wait(&pointer->input_event);
        if (new_event != NULL){
            //debug("Get event uid : %d", new_event->uid);
            if (new_event->uid == _EVENT_STOP_POINTER_UID){
                free(new_event);
                break;
            }
            reachable_transition = get_reachable_condition(pointer->current_step->transitions, new_event);
            if (reachable_transition != NULL){
                ret_step = start_step(pointer, reachable_transition->next_step, new_event);
                free(new_event);
                continue;
            }
        }
        free(new_event);
        // Condition
    }
    return NULL;
}



/*! Start a step function with the appropriate context
 *
 * Create the appropriate context and start the step function with it
 *
 */
struct fsm_step *start_step(struct fsm_pointer *pointer, struct fsm_step *step, struct fsm_event *event) {
    struct fsm_context init_context = {
            .event = *event,
            .fnct_args = step->args,
    };
    pthread_mutex_lock(&pointer->mutex);
    pointer->current_step = step;
    pthread_mutex_unlock(&pointer->mutex);
    //debug("Step by event %u", event.uid);
    return step->fnct(&init_context);
}

struct fsm_event *signal_fsm_pointer_of_event(struct fsm_pointer *pointer, struct fsm_event event) {
    return push_back_fsm_event_queue(&pointer->input_event, event);
}

unsigned short destroy_pointer(struct fsm_pointer *pointer) {
    join_pointer(pointer);
    free(pointer);
    pointer = NULL;
}

struct fsm_event generate_event(short event_uid, void *args) {
    struct fsm_event event = {
            .uid = event_uid,
            .args = args
    };
    return event;
}

unsigned short destroy_all_steps() {
    while(_all_steps_created->first != NULL){
        struct fsm_step *step = (struct fsm_step *)pop_front_fsm_queue(_all_steps_created);
        free(step);
    }
    return 0;
}

void join_pointer(struct fsm_pointer *pointer) {
    pthread_mutex_lock(&pointer->mutex);
    if(pointer->running == 1) {
        signal_fsm_pointer_of_event(pointer, generate_event(_EVENT_STOP_POINTER_UID, NULL));
        pthread_mutex_unlock(&pointer->mutex);
        pthread_join(pointer->thread, NULL);
        pthread_mutex_lock(&pointer->mutex);
        pointer->running = 0;
    }
    cleanup_fsm_queue(&pointer->input_event);
    pthread_mutex_unlock(&pointer->mutex);
}
