//
// Created by olivier on 03/08/15.
//

#ifndef FSM_NEW_FSM_H
#define FSM_NEW_FSM_H

/*!
 * \file fsm.h
 * \brief API main header for the C FSM API
 * \author Olivier Radisson
 * \version 0.1
 */

#include "events.h"
#include "pthread.h"
#include "fsm_queue.h"

#define FSM_STATE_STOPPED  0
#define FSM_STATE_RUNNING  1
#define FSM_STATE_STARTING 2
#define FSM_STATE_CLOSING  3

struct fsm_event
{
    char uid[MAX_EVENT_UID_LEN];
    void * args;
};

struct fsm_context{
    struct fsm_event * event;
    struct fsm_pointer *pointer;
};

struct fsm_transition {
    char event_uid[MAX_EVENT_UID_LEN];
    struct fsm_step *next_step;
};

struct fsm_step{
    void * (*fnct)(struct fsm_context *);
    void * args;
    struct fsm_queue * transitions;
};

struct fsm_pointer{
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond_event;
    struct fsm_queue input_event;
    struct fsm_step * current_step;
    unsigned short running;
};

typedef struct fsm_pointer fsm_pointer;
typedef struct fsm_step fsm_step;
typedef struct fsm_event fsm_event;
typedef struct fsm_transition fsm_transition;
typedef struct fsm_context fsm_context;


struct fsm_pointer *fsm_create_pointer();
struct fsm_event *fsm_generate_event(char *event_uid, void *args);
void fsm_start_pointer(struct fsm_pointer *_pointer, struct fsm_step *init_step);
void fsm_join_pointer(struct fsm_pointer *pointer);
void *fsm_pointer_loop(void *pointer);

/*! Create a step pointer based on a callback function and generic void pointer as argument.
 *      @param  fnct Callback function which be called when entering step.
 *      @param  args Pointer which be passed to the callback function. Can be set to NULL.
 *
 *  @return Pointer to the new created step.
 *
 *  @note Use \c malloc for the step allocation : you should free it at the end of it's usage
 *  @note The fsm_delete_all_steps() function allow you the free all allocated steps
 */
struct fsm_step *fsm_create_step(void *(*fnct)(struct fsm_context *), void *args);

/*! Connect two step with an event by creating a transition.
 *      @param from Transition start point.
 *      @param to Transition end point.
 *      @param event_uid UID of the event linking start point to end point.
 *
 * Create a transition for the given event_uid referring to the \a to step pointer and
 * attach it to the \a from step pointer.
 *
 * Example :
 * @code{.c}
 * fsm_step *step_from = fsm_create_step(fsm_null_callback, NULL);
 * fsm_step *step_to = fsm_create_step(fsm_null_callback, NULL);
 * fsm_connect_step(step_from, step_to, "GO");
 * @endcode
 *
 */
void fsm_connect_step(struct fsm_step *from, struct fsm_step *to, char *event_uid);
struct fsm_step *fsm_start_step(struct fsm_pointer *pointer, struct fsm_step *step, struct fsm_event *event);
struct fsm_event *fsm_signal_pointer_of_event(struct fsm_pointer *pointer, struct fsm_event *event);
unsigned short fsm_delete_pointer(struct fsm_pointer *pointer);
unsigned short fsm_delete_all_steps();
int fsm_wait_step_mstimeout(struct fsm_pointer *pointer, struct fsm_step *step, unsigned int mstimeout);
int fsm_wait_step_blocking(struct fsm_pointer *pointer, struct fsm_step *step);
int fsm_wait_leaving_step_blocking(struct fsm_pointer *pointer, struct fsm_step *step);
int fsm_wait_leaving_step_mstimeout(struct fsm_pointer *pointer, struct fsm_step *step, unsigned int mstimeout);


void * fsm_null_callback(struct fsm_context *context);


#endif //FSM_NEW_FSM_H
