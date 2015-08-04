//
// Created by olivier on 03/08/15.
//

#ifndef FSM_1_NEW_FSM_H
#define FSM_1_NEW_FSM_H

#include "pthread.h"

struct fsm_event
{
    short uid;
    void * args;
};

struct fsm_context{
    struct fsm_event event;
    void * fnct_args;
};

struct fsm_trans{
    short event_uid;
    const struct fsm_step *next_step;
};

struct fsm_step{
    void (*fnct)(const struct fsm_context *);
    void * args;
    struct fsm_trans transition;
};

struct fsm_pointer{
    pthread_t thread;
    pthread_mutex_t mutex_event;
    pthread_cond_t cond_event;
    struct fsm_event input_event;
    struct fsm_step current_step;
    unsigned short started;
};


struct fsm_pointer * create_pointer(struct fsm_step first_step);
void start_pointer(struct fsm_pointer *_pointer);
void * pointer_loop(void *pointer);
struct fsm_step create_step(void (*fnct)(const struct fsm_context *), void * args);
void connect_step(struct fsm_step *from, struct fsm_step *to, short event_uid);
void start_step(struct fsm_step step, struct fsm_event event);


void callback(const struct fsm_context *context);


extern struct fsm_event START_EVENT;
extern struct fsm_event _END_POINTER;
extern struct fsm_event _NONE_EVENT;
extern struct fsm_trans TRANS_ENDPOINT;


#endif //FSM_1_NEW_FSM_H
