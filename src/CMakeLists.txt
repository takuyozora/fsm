add_library(fsm_queue fsm_queue.c fsm_queue.h fsm_time.h fsm_time.c)
add_library(fsm fsm.h fsm.c fsm_queue.c fsm_queue.h fsm_time.h fsm_time.c)