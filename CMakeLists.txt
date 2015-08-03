cmake_minimum_required(VERSION 3.2)
project(fsm_1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall  -lpthread -D_REENTRANT ") #-Werror")

set(SOURCE_FILES main.c)
add_executable(fsm_1 ${SOURCE_FILES} new_fsm.h new_fsm.c)#fsm.h fsm.c)