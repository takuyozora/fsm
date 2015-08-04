//
// Created by olivier on 03/08/15.
//



#ifndef FSM_1_FSM_H
#define FSM_1_FSM_H

//#include "pthread.h"

struct test_in {
    int a;
    int b;
};

struct event{
    unsigned int x;
    char y;
};

struct mxd_event{
    int x;
    double y;
    char z;
};

struct context{
    const struct event *event;
    const char *args;
};

struct step {
    void (*fnct)(const struct context *);
    const char args;
    const struct transition *transition;
};

struct transition
{
    const unsigned int event_uid;
    struct step *next_step;
};
/*
struct pointer
{
    struct event *input;
    pthread_t thread;
    pthread_cond_t cond_new_input;
    pthread_mutex_t mutex;
};

struct init_pointer{
    struct step *first_step;
    struct pointer *pointer;
};*/

//int start_fsm(struct pointer *pointer, struct step step);

void _callback(const struct event *event, const char *args);

void callback(const struct context *context);

//void pointer_event_signal(struct event *event, struct pointer *pointer);

//void * pointer_loop(void *init_data);


extern struct event START_EVENT;
extern struct event _END_POINTER;




#endif //FSM_1_FSM_H
