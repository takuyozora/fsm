//
// Created by olivier on 04/08/15.
//

#ifndef FSM_FSM_EVENT_QUEUE_H
#define FSM_FSM_EVENT_QUEUE_H

#include "fsm.h"
#include "fsm_queue.h"

struct fsm_event * push_back_fsm_event_queue(struct fsm_queue *queue, struct fsm_event event);
struct fsm_event * pop_front_fsm_event_queue(struct fsm_queue *queue);
struct fsm_event * get_event_or_wait(struct fsm_queue *queue);

#endif //FSM_FSM_EVENT_QUEUE_H


