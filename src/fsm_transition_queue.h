//
// Created by olivier on 04/08/15.
//

#ifndef FSM_1_FSM_TRANSITION_QUEUE_H
#define FSM_1_FSM_TRANSITION_QUEUE_H

#include "fsm_queue.h"
#include "fsm.h"

struct fsm_transition * push_back_fsm_transition_queue(struct fsm_queue *queue, struct fsm_transition *transition);
struct fsm_transition * get_reachable_condition(struct fsm_queue *queue, struct fsm_event *event);

#endif //FSM_1_FSM_TRANSITION_QUEUE_H
