//
// Created by olivier on 03/08/15.
//

#include <stdlib.h>
#include "fsm.h"
#include "fsm_event_queue.h"
#include "stdio.h"
#include "debug.h"


struct fsm_trans TRANS_ENDPOINT = {0, NULL};
struct fsm_event START_EVENT = {1, NULL};
struct fsm_event _END_POINTER = {-1, NULL};
struct fsm_event _NONE_EVENT = {-2, NULL};


/*! Just a example callback function
 */
void * callback(const struct fsm_context *context) {
    //log_info("Callback : event uid : %d \n", context->event.uid);
    return NULL;
}

/*! Simplify step creation.
 *
 *  Create a step from a function and a void pointer as argument for it.
 *  This function initialize transition to TRANS_ENDPOINT, a null transition
 *
 */
struct fsm_step create_step(void * (*fnct)(const struct fsm_context *), void *args) {
    struct fsm_step result = {
            .fnct = fnct,
            .args = args,
            .transition = TRANS_ENDPOINT,
    };
    return result;
}

/*! Connect two step with an event by creating a transition
 *
 * Create a transition for the given event which refer to the \a to step and
 * attach it to the \a from step
 *
 */
void connect_step(struct fsm_step *from, struct fsm_step *to, short event_uid) {
    struct fsm_trans transition = {
            .event_uid = event_uid,
            .next_step = to,
    };
    from->transition = transition;
}

/*! Create a pointer wich is the main part of a FSM
 *
 * Create a pointer with \a first_step as current step. Do not start it and just init other
 * thread vars
 *
 */
struct fsm_pointer * create_pointer(struct fsm_step first_step)
{
    struct fsm_pointer *pointer = malloc(sizeof(struct fsm_pointer));
    pointer->thread = 0;
    pthread_mutex_init(&pointer->mutex_event, NULL);
    pthread_cond_init(&pointer->cond_event, NULL);
    pointer->input_event = create_fsm_queue();
    pointer->current_step = first_step;
    pointer->started = 0;
    return pointer;
}

/*! Start the given pointer in a thread
 *
 * Check if the pointer isn't already started, if not start it in a new thread
 *
 */
void start_pointer(struct fsm_pointer *pointer) {
    if ( pointer->started != 0 ) {
        log_err("CRITICAL : A pointer must be started only once");
        return;
    }
    pthread_create(&(pointer->thread), NULL, &pointer_loop, (void *)pointer);
    pointer->started = 1;
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
    struct fsm_event * new_event = 0;
    ret_step = start_step(pointer->current_step, START_EVENT);
    while (1){
        if(ret_step != NULL){
            ret_step = start_step(*ret_step, _NONE_EVENT);
            continue;
        }
        new_event = get_event_or_wait(&pointer->input_event);
        if (new_event != NULL){
            //debug("Get event uid : %d", new_event->uid);
            if (new_event->uid == _END_POINTER.uid){
                free(new_event);
                break;
            }
            if (new_event->uid == pointer->current_step.transition.event_uid){
                ret_step = start_step(*pointer->current_step.transition.next_step, *new_event);
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
struct fsm_step * start_step(struct fsm_step step, struct fsm_event event) {
    struct fsm_context init_context = {
            .event = event,
            .fnct_args = step.args,
    };
    //debug("Step by event %u", event.uid);
    return step.fnct(&init_context);
}

struct fsm_event *signal_fsm_pointer_of_event(struct fsm_pointer *pointer, struct fsm_event *event) {
    return push_back_fsm_event_queue(&pointer->input_event, event);
}

unsigned short destroy_pointer(struct fsm_pointer *pointer) {
    cleanup_fsm_queue(&pointer->input_event);
    free(pointer);
}
