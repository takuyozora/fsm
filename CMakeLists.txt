cmake_minimum_required(VERSION 3.2)
project(fsm)
enable_testing()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wno-unused-variable  -lpthread -D_REENTRANT -std=c11 ")
add_subdirectory(test)
add_subdirectory(src)
set(SOURCE_FILES main.c)

add_executable(fsm_main ${SOURCE_FILES} src/fsm.h src/fsm.c src/fsm_queue.h src/fsm_queue.c src/fsm_event_queue.h src/fsm_event_queue.c src/debug.h src/fsm_transition_queue.h src/fsm_transition_queue.c)#fsm.h fsm.c)