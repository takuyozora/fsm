//
// Created by olivier on 03/08/15.
//

#include <stdlib.h>
#include "new_fsm.h"
#include "stdio.h"


struct fsm_trans TRANS_ENDPOINT = {0, NULL};
struct fsm_event START_EVENT = {1, NULL};
struct fsm_event _END_POINTER = {-1, NULL};
struct fsm_event _NONE_EVENT = {-2, NULL};


/*! Just a example callback function
 */
void callback(const struct fsm_context *context) {
    printf("Callback : event uid : %d \n", context->event.uid);
}

/*! Simplify step creation.
 *
 *  Create a step from a function and a void pointer as argument for it.
 *  This function initialize transition to TRANS_ENDPOINT, a null transition
 *
 */
struct fsm_step create_step(void (*fnct)(const struct fsm_context *), void *args) {
    struct fsm_step result = {
            .fnct = (*fnct),
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
struct fsm_pointer create_pointer(struct fsm_step first_step)
{
    struct fsm_pointer pointer = {
            .thread = NULL,
            .mutex_event = PTHREAD_MUTEX_INITIALIZER,
            .cond_event = PTHREAD_COND_INITIALIZER,
            .input_event = _NONE_EVENT,
            .current_step = first_step,
            .started = 0
    };
    return pointer;
}

/*! Start the given pointer in a thread
 *
 * Check if the pointer isn't already started, if not start it in a new thread
 *
 */
void start_pointer(struct fsm_pointer *pointer) {
    if ( pointer->started != 0 ){
        printf("CRITICAL : A pointer must be started only once");
        return;
    }
    pthread_create(&pointer->thread, NULL, &pointer_loop, (void *) pointer);
    pointer->started = 1;
}

/*struct fsm_pointer *_pt;
    _pt = malloc(sizeof(struct fsm_pointer));
    (*_pt).thread = pointer->thread;
    (*_pt).current_step = pointer->current_step;
    (*_pt).cond_event = pointer->cond_event;
    (*_pt).mutex_event = pointer->mutex_event;
    (*_pt).input_event = pointer->input_event;
    (*_pt).started = pointer->started;*/

/*! Main loop for the pointer thread, run step and wait for event
 *
 * The main loop of a pointer thread execute a step and wait for a transition
 * when this one ended.
 *
 * This function shouldn't be called from an other place that start_pointer function
 *
 * NOT FULLY IMPLEMENTED
 */
void *pointer_loop(void *_pointer) {
    struct fsm_pointer *pointer = _pointer;
    start_step(pointer->current_step, START_EVENT);
    return NULL;
}

/*! Start a step function with the appropriate context
 *
 * Create the appropriate context and start the step function with it
 *
 */
void start_step(struct fsm_step step, struct fsm_event event) {
    struct fsm_context init_context = {
            .event = event,
            .fnct_args = step.args,
    };
    step.fnct(&init_context);
}
