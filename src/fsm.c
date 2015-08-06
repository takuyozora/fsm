//
// Created by olivier on 03/08/15.
//

#include <stdlib.h>
#include "fsm.h"
#include "fsm_event_queue.h"
#include "fsm_transition_queue.h"
#include "stdio.h"
#include "debug.h"
#include <sys/time.h>

// Global var to keep a trace of all steps created in order to free them at the end
static struct fsm_queue *_all_steps_created = NULL;


/*! Simplify step creation.
 *
 *  Create a step from a function and a void pointer as argument for it.
 *  This function initialize transition to TRANS_ENDPOINT, a null transition
 *
 */
struct fsm_step * create_step(void *(*fnct)(struct fsm_context *), void *args) {
    if (_all_steps_created == NULL){
        _all_steps_created = create_fsm_queue_pointer();
    }
    struct fsm_step *step = malloc(sizeof(struct fsm_step));
    step = push_back_fsm_queue_more(_all_steps_created, (void *)step, sizeof(*step), 0);
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
void connect_step(struct fsm_step *from, struct fsm_step *to, char *event_uid) {
    struct fsm_transition transition = {
            .next_step = to,
    };
    strcpy(transition.event_uid, event_uid);
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
    pointer->running = FSM_STATE_STOPPED;
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
    if ( pointer->running != FSM_STATE_STOPPED ) {
        log_err("CRITICAL : A pointer can't be running twice a time");
        return;
    }
    pointer->current_step = init_step;
    pointer->running = FSM_STATE_STARTING;
    pthread_create(&(pointer->thread), NULL, &pointer_loop, (void *)pointer);
    while(pointer->running == FSM_STATE_STARTING){
        pthread_cond_wait(&pointer->cond_event, &pointer->mutex);
    }
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
    struct fsm_event * new_event = generate_event(_EVENT_START_POINTER_UID, NULL);
    struct fsm_step * ret_step = start_step(pointer, pointer->current_step, new_event); // Allow to start the first step without transition
    struct fsm_transition * reachable_transition = NULL;
    while (1){
        if(pointer->running != FSM_STATE_RUNNING){
            free(new_event);
            break;
        }
        if(ret_step != NULL){
            ret_step = start_step(pointer, ret_step, new_event);
            continue;
        }
        if(pointer->current_step->transitions->first != NULL){ // Check if there isn't direct transition
            if(strcmp(((struct fsm_transition *)(pointer->current_step->transitions->first->value))->event_uid,
                _EVENT_DIRECT_TRANSITION) == 0){
                // Then we direct go to next step
                ret_step = start_step(pointer, ((struct fsm_transition *)
                        (pointer->current_step->transitions->first->value))->next_step, new_event);
                continue;
            }
        }
        free(new_event);
        new_event = get_event_or_wait(&pointer->input_event);
        if (new_event != NULL){
            //debug("Get event uid : %d", new_event->uid);
            if (strcmp(new_event->uid, _EVENT_STOP_POINTER_UID) == 0){
                free(new_event);
                break;
            }
            reachable_transition = get_reachable_condition(pointer->current_step->transitions, new_event);
            if (reachable_transition != NULL){
                ret_step = start_step(pointer, reachable_transition->next_step, new_event);
                continue;
            }
        }
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
            .event = event,
            .pointer = pointer,
    };
    pthread_mutex_lock(&pointer->mutex);
    pointer->current_step = step;
    if(pointer->running == FSM_STATE_STARTING) {
        pointer->running = FSM_STATE_RUNNING;
    }
    pthread_cond_broadcast(&pointer->cond_event);
    pthread_mutex_unlock(&pointer->mutex);
    return step->fnct(&init_context);
}

struct fsm_event *signal_fsm_pointer_of_event(struct fsm_pointer *pointer, struct fsm_event *event) {
    return push_back_fsm_event_queue(&pointer->input_event, event);
}

unsigned short destroy_pointer(struct fsm_pointer *pointer) {
    join_pointer(pointer);
    free(pointer);
    pointer = NULL;
    return 0;
}

struct fsm_event * generate_event(char *event_uid, void *args) {
    struct fsm_event *event = malloc(sizeof(struct fsm_event));
    strcpy(event->uid, event_uid);
    event->args = args;
    return event;
}

unsigned short destroy_all_steps() {
    while(_all_steps_created->first != NULL){
        struct fsm_step *step = (struct fsm_step *)pop_front_fsm_queue(_all_steps_created);
        destory_fsm_queue_pointer(step->transitions);
        free(step);
    }
    destory_fsm_queue_pointer(_all_steps_created);
    _all_steps_created = NULL;
    return 0;
}

void join_pointer(struct fsm_pointer *pointer) {
    pthread_mutex_lock(&pointer->mutex);
    if(pointer->running == FSM_STATE_RUNNING) {
        signal_fsm_pointer_of_event(pointer, generate_event(_EVENT_STOP_POINTER_UID, NULL));
        pointer->running = FSM_STATE_CLOSING;
        pthread_mutex_unlock(&pointer->mutex);
        pthread_join(pointer->thread, NULL);
        pthread_mutex_lock(&pointer->mutex);
        pointer->running = FSM_STATE_STOPPED;
    }
    cleanup_fsm_queue(&pointer->input_event);
    pthread_mutex_unlock(&pointer->mutex);
}

void *fsm_null_callback(struct fsm_context *context) {
    return NULL;
}

int _fsm_wait_step_mstimeout(struct fsm_pointer *pointer, struct fsm_step *step, unsigned int mstimeout, char leave) {
    struct timeval tv;
    struct timespec ts;
    int rc = 0;

    gettimeofday(&tv, NULL);
    ts.tv_sec = time(NULL) + mstimeout / 1000;
    ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (mstimeout % 1000);
    ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
    ts.tv_nsec %= (1000 * 1000 * 1000);

    pthread_mutex_lock(&pointer->mutex);
    while ( (pointer->current_step == step && leave == 1) || (pointer->current_step != step && leave == 0) ){
        rc = pthread_cond_timedwait(&pointer->cond_event, &pointer->mutex, &ts);
        if (rc == ETIMEDOUT){
            break;
        }
    }
    pthread_mutex_unlock(&pointer->mutex);
    return rc;
}

int _fsm_wait_step_blocking(struct fsm_pointer *pointer, struct fsm_step *step, char leave) {
    int rc = 0;
    pthread_mutex_lock(&pointer->mutex);
    while ( (pointer->current_step == step && leave == 1) || (pointer->current_step != step && leave == 0) ){
        rc = pthread_cond_wait(&pointer->cond_event, &pointer->mutex);
    }
    pthread_mutex_unlock(&pointer->mutex);
    return rc;
}


int fsm_wait_step_blocking(struct fsm_pointer *pointer, struct fsm_step *step){
    return _fsm_wait_step_blocking(pointer, step, 0);
}

int fsm_wait_leaving_step_blocking(struct fsm_pointer *pointer, struct fsm_step *step){
    return _fsm_wait_step_blocking(pointer, step, 1);
}

int fsm_wait_step_mstimeout(struct fsm_pointer *pointer, struct fsm_step *step, unsigned int mstimeout){
    return _fsm_wait_step_mstimeout(pointer, step, mstimeout, 0);
}

int fsm_wait_leaving_step_mstimeout(struct fsm_pointer *pointer, struct fsm_step *step, unsigned int mstimeout){
    return _fsm_wait_step_mstimeout(pointer, step, mstimeout, 1);
}

