//
// Created by olivier on 03/08/15.
//

//#include "pthread.h"
#include "fsm.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct event START_EVENT = {0, 0};
struct event _END_POINTER = {10, 0};


void _callback(const struct event *event, const char *args){
    printf("Hello world ! %u, %c, %c", event->x, event->y, *args);
}

void callback(const struct context *context){
    printf("Hello world ! %u, %c, %c", context->event->x, context->event->y, *context->args);
}
/*
int start_fsm(struct pointer *pointer, struct step step) {
    struct init_pointer init = {&step, pointer};
    return pthread_create(&pointer->thread, NULL, &pointer_loop, &init);
}

void pointer_event_signal(struct event *event, struct pointer *pointer) {
    pthread_mutex_lock(&pointer->mutex);
    *pointer->input = *event;
    pthread_mutex_unlock(&pointer->mutex);
    pthread_cond_signal(&pointer->cond_new_input);
}

void * pointer_loop(void *init_data) {
    printf("Start pointer loop");
    struct init_pointer *data = (struct init_pointer *) init_data;
    struct step *current_step = data->first_step;
    //struct event *new_event = malloc(sizeof(struct event));
    struct context context = {&START_EVENT, &current_step->args};
    while (1) {
        printf("New loop pointer ..");
        int i = 0;
        current_step->fnct(&context);
        pthread_mutex_lock(&data->pointer->mutex);
        pthread_cond_wait(&data->pointer->cond_new_input, &data->pointer->mutex);
        printf("UID : %u", data->pointer->input->uid);
        pthread_mutex_unlock(&data->pointer->mutex);
        memcpy(new_event, data->pointer->input, sizeof(struct event));

        if (new_event->uid == _END_POINTER.uid) {
            break;
        } else if (new_event->uid == current_step->transition->event_uid) {
            current_step = current_step->transition->next_step;
            context.event = new_event;
            context.args = &current_step->args;
        }
        break;
    }
    printf("End pointer loop");
    return NULL;
}
*/