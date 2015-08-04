cmake_minimum_required(VERSION 3.2)
project(fsm_1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall  -lpthread -D_REENTRANT ") #-Werror")

set(SOURCE_FILES main.c)
add_executable(fsm_1 ${SOURCE_FILES} fsm.h fsm.c fsm_queue.h fsm_queue.c fsm_event_queue.h fsm_event_queue.c debug.h)#fsm.h fsm.c)