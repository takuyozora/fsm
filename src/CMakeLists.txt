add_library(fsm_queue fsm_queue.c fsm_queue.h)
add_library(fsm fsm.h fsm.c fsm_queue.c fsm_queue.h )