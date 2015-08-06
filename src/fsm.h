//
// Created by olivier on 03/08/15.
//

#ifndef FSM_NEW_FSM_H
#define FSM_NEW_FSM_H

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
    void * fnct_args; // Depreciate : should use pointer->current_step->fnct_args
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


struct fsm_pointer *create_pointer();
struct fsm_event * generate_event(char *event_uid, void *args);
void start_pointer(struct fsm_pointer *_pointer, struct fsm_step *init_step);
void join_pointer(struct fsm_pointer *pointer);
void * pointer_loop(void *pointer);
struct fsm_step * create_step(void *(*fnct)(struct fsm_context *), void *args);
void connect_step(struct fsm_step *from, struct fsm_step *to, char *event_uid);
struct fsm_step *start_step(struct fsm_pointer *pointer, struct fsm_step *step, struct fsm_event *event);
struct fsm_event * signal_fsm_pointer_of_event(struct fsm_pointer *pointer, struct fsm_event *event);
unsigned short destroy_pointer(struct fsm_pointer *pointer);
unsigned short destroy_all_steps();
int fsm_wait_step_mstimeout(struct fsm_pointer *pointer, struct fsm_step *step, unsigned int mstimeout);
int fsm_wait_step_blocking(struct fsm_pointer *pointer, struct fsm_step *step);
int fsm_wait_leaving_step_blocking(struct fsm_pointer *pointer, struct fsm_step *step);
int fsm_wait_leaving_step_mstimeout(struct fsm_pointer *pointer, struct fsm_step *step, unsigned int mstimeout);


void * fsm_null_callback(struct fsm_context *context);


#endif //FSM_NEW_FSM_H
