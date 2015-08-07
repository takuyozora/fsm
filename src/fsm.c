//
// Created by olivier on 03/08/15.
//

#include <stdlib.h>
#include "fsm.h"
#include "fsm_event_queue.h"
#include "fsm_transition_queue.h"
#include "stdio.h"
#include "debug.h"

// Global var to keep a trace of all steps created in order to free them at the end
static struct fsm_queue *_all_steps_created = NULL;


/*! Start a step function with the appropriate context
 *      @param pointer Pointer to the fsm_pointer entering to the given step
 *      @param step Pointer to the new fsm_step to run
 *      @param event Pointer to the event which have triggered the transition
 *
 * @return Return the output of the fsm_step callback function
 * @retval NULL in most cases
 * @retval fsm_step pointer to the next step to go
 *
 * @note If the return is not a NULL pointer the fsm_pointer will directly go to this step without looking for
 * @note any transition.
 *
 */
struct fsm_step *fsm_start_step(struct fsm_pointer *pointer, struct fsm_step *step, struct fsm_event *event) {
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


/*! Main loop for the pointer thread, run step and wait for events
 *      @param pointer Generic void pointer to the fsm_pointer which will be managed by the loop
 *
 * The main loop of a pointer thread execute a step and wait for a transition
 * when this one ended.
 *
 * @warning This function shouldn't be called from an other place that fsm_start_pointer function
 *
 */
void *fsm_pointer_loop(void *_pointer) {
    struct fsm_pointer * pointer = _pointer;
    struct fsm_event * new_event = fsm_generate_event(_EVENT_START_POINTER_UID, NULL);
    struct fsm_step * ret_step = fsm_start_step(pointer, pointer->current_step, new_event); // Allow to start the first step without transition
    struct fsm_transition * reachable_transition = NULL;
    while (1){
        if(pointer->running != FSM_STATE_RUNNING){
            free(new_event);
            break;
        }
        if(ret_step != NULL){
            ret_step = fsm_start_step(pointer, ret_step, new_event);
            continue;
        }
        if(pointer->current_step->transitions->first != NULL){ // Check if there isn't direct transition
            if(strcmp(((struct fsm_transition *)(pointer->current_step->transitions->first->value))->event_uid,
                      _EVENT_DIRECT_TRANSITION) == 0){
                // Then we direct go to next step
                ret_step = fsm_start_step(pointer, ((struct fsm_transition *)
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
                ret_step = fsm_start_step(pointer, reachable_transition->next_step, new_event);
                continue;
            }
        }
        // Condition
    }
    return NULL;
}

/*! This function delete indeed a step
 *      @param step Pointer to the fsm_step to delete
 *
 *  @note fsm_queue transitions from the step is also delete
 * */
void _fsm_delete_a_step(fsm_step *step){
    fsm_queue_delete_queue_pointer(step->transitions);
    free(step);
}


struct fsm_step *fsm_create_step(void *(*fnct)(struct fsm_context *), void *args) {
    if (_all_steps_created == NULL){
        // Init _all_steps_created if it's still a NULL pointer
        _all_steps_created = create_fsm_queue_pointer();
    }
    struct fsm_step *step = malloc(sizeof(struct fsm_step));
    // Add the new step into the _all_steps_created to free after
    step = fsm_queue_push_back_more(_all_steps_created, (void *) step, sizeof(*step), 0);
    step->fnct = fnct;
    step->args = args;
    step->transitions = create_fsm_queue_pointer();
    return step;
}


void fsm_connect_step(struct fsm_step *from, struct fsm_step *to, char *event_uid) {
    struct fsm_transition transition = {
            .next_step = to,
    };
    // Copy the event's UID to the transition
    strcpy(transition.event_uid, event_uid);
    // Add transition to the from transition queue
    push_back_fsm_transition_queue(from->transitions, &transition);
}


struct fsm_pointer *fsm_create_pointer()
{
    struct fsm_pointer *pointer = malloc(sizeof(struct fsm_pointer));
    pointer->thread = 0;
    // Init thread mutex and condition
    pthread_mutex_init(&pointer->mutex, NULL);
    pthread_cond_init(&pointer->cond_event, NULL);
    //
    pointer->input_event = create_fsm_queue();
    pointer->current_step = NULL;
    pointer->running = FSM_STATE_STOPPED;
    return pointer;
}


unsigned short fsm_start_pointer(struct fsm_pointer *pointer, struct fsm_step *init_step) {
    pthread_mutex_lock(&pointer->mutex);
    if ( pointer->running != FSM_STATE_STOPPED ) {
        log_err("A pointer can't be started if it's not stopped");
        return FSM_ERR_NOT_STOPPED;
    }
    pointer->current_step = init_step;
    pointer->running = FSM_STATE_STARTING;
    pthread_create(&(pointer->thread), NULL, &fsm_pointer_loop, (void *) pointer);
    // Waiting for the pointer to start his first step
    while(pointer->running == FSM_STATE_STARTING){
        pthread_cond_wait(&pointer->cond_event, &pointer->mutex);
    }
    pthread_mutex_unlock(&pointer->mutex);
    return 0;
}



void fsm_signal_pointer_of_event(struct fsm_pointer *pointer, struct fsm_event *event) {
    push_back_fsm_event_queue(&pointer->input_event, event);
}

void fsm_delete_pointer(struct fsm_pointer *pointer) {
//    if(pointer == NULL) {
//        log_warn("Asking to delete a NULL fsm_pointer");
//        return FSM_ERR_NULL_POINTER;
//    }
    fsm_join_pointer(pointer);
    free(pointer);
}

struct fsm_event *fsm_generate_event(char *event_uid, void *args) {
    struct fsm_event *event = malloc(sizeof(struct fsm_event));
    // Copy the given event UID to the new allocated event
    strcpy(event->uid, event_uid);
    event->args = args;
    return event;
}

void fsm_delete_all_steps() {
    while(_all_steps_created->first != NULL){
        struct fsm_step *step = (struct fsm_step *) fsm_queue_pop_front(_all_steps_created);
        _fsm_delete_a_step(step);
    }
    fsm_queue_delete_queue_pointer(_all_steps_created);
    _all_steps_created = NULL;
}

void fsm_join_pointer(struct fsm_pointer *pointer) {
    if (pointer == NULL){
        log_warn("Asking to join a NULL fsm_pointer");
        return;
    }
    pthread_mutex_lock(&pointer->mutex);
    if(pointer->running == FSM_STATE_RUNNING) {
        // Add signal to close in the pointer input_event queue
        fsm_signal_pointer_of_event(pointer, fsm_generate_event(_EVENT_STOP_POINTER_UID, NULL));
        // Set pointer running step to closing in case the pointer do not whatch his transitions (because of a direct loop for example)
        pointer->running = FSM_STATE_CLOSING;
        pthread_mutex_unlock(&pointer->mutex);
        pthread_join(pointer->thread, NULL);
        pthread_mutex_lock(&pointer->mutex);
        pointer->running = FSM_STATE_STOPPED;
    }
    fsm_queue_cleanup(&pointer->input_event);
    pthread_mutex_unlock(&pointer->mutex);
}

void *fsm_null_callback(struct fsm_context *context) {
    return NULL;
}

int _fsm_wait_step_mstimeout(struct fsm_pointer *pointer, struct fsm_step *step, unsigned int mstimeout, char leave) {
    struct timeval tv;
    struct timespec ts;
    int rc = 0;

    // Creating correct time variable for an absolute time from the given time in ms
    gettimeofday(&tv, NULL);
    ts.tv_sec = time(NULL) + mstimeout / 1000;
    ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (mstimeout % 1000);
    ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
    ts.tv_nsec %= (1000 * 1000 * 1000);

    pthread_mutex_lock(&pointer->mutex);
    // Wait that the given step become the current one or the opposite according to the leave value
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
    pthread_mutex_lock(&pointer->mutex);
    // Wait that the given step become the current one or the opposite according to the leave value
    while ( (pointer->current_step == step && leave == 1) || (pointer->current_step != step && leave == 0) ){
        pthread_cond_wait(&pointer->cond_event, &pointer->mutex);
    }
    pthread_mutex_unlock(&pointer->mutex);
}


void fsm_wait_step_blocking(struct fsm_pointer *pointer, struct fsm_step *step){
    _fsm_wait_step_blocking(pointer, step, 0);
}

void fsm_wait_leaving_step_blocking(struct fsm_pointer *pointer, struct fsm_step *step){
    _fsm_wait_step_blocking(pointer, step, 1);
}

int fsm_wait_step_mstimeout(struct fsm_pointer *pointer, struct fsm_step *step, unsigned int mstimeout){
    return _fsm_wait_step_mstimeout(pointer, step, mstimeout, 0);
}

int fsm_wait_leaving_step_mstimeout(struct fsm_pointer *pointer, struct fsm_step *step, unsigned int mstimeout){
    return _fsm_wait_step_mstimeout(pointer, step, mstimeout, 1);
}

void fsm_delete_a_step(fsm_step *step) {
    if(fsm_queue_get_elem(_all_steps_created, step) != NULL){
        // We are sure that the step exist and we've removed it from _all_created_steps
        // Now we can delete it safely
        _fsm_delete_a_step(step);
    }
}
