//
// Created by olivier on 04/08/15.
//

#include "fsm_event_queue.h"

struct fsm_event *pop_front_fsm_event_queue(struct fsm_queue *queue){
    return (struct fsm_event *)pop_front_fsm_queue(queue);
}

struct fsm_event * push_back_fsm_event_queue(struct fsm_queue *queue, struct fsm_event *event) {
    return (struct fsm_event *)push_back_fsm_queue(queue, (void *)event, sizeof(*event));
}
