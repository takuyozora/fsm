//
// Created by olivier on 03/08/15.
//

#ifndef FSM_NEW_FSM_H
#define FSM_NEW_FSM_H

#define _EVENT_STOP_POINTER_UID -1

#include "pthread.h"
#include "fsm_queue.h"

struct fsm_event
{
    short uid;
    void * args;
};

struct fsm_context{
    struct fsm_event event;
    void * fnct_args;
};

struct fsm_transition {
    short event_uid;
    struct fsm_step *next_step;
};

struct fsm_step{
    void * (*fnct)(const struct fsm_context *);
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
struct fsm_event generate_event(short event_uid, void *args);
void start_pointer(struct fsm_pointer *_pointer, struct fsm_step *init_step);
void join_pointer(struct fsm_pointer *pointer);
void * pointer_loop(void *pointer);
struct fsm_step * create_step(void *(*fnct)(const struct fsm_context *), void *args);
void connect_step(struct fsm_step *from, struct fsm_step *to, short event_uid);
struct fsm_step *start_step(struct fsm_pointer *pointer, struct fsm_step *step, struct fsm_event *event);
struct fsm_event * signal_fsm_pointer_of_event(struct fsm_pointer *pointer, struct fsm_event event);
unsigned short destroy_pointer(struct fsm_pointer *pointer);
unsigned short destroy_all_steps();


void * callback(const struct fsm_context *context);


extern struct fsm_event START_EVENT;
extern struct fsm_event _END_POINTER;
extern struct fsm_event _NONE_EVENT;
extern struct fsm_transition TRANS_ENDPOINT;

//static struct fsm_queue *_all_steps_created;


#endif //FSM_NEW_FSM_H