cmake_minimum_required(VERSION 3.0)
project(fsm)
enable_testing()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wno-unused-variable  -lpthread -D_REENTRANT -std=gnu11 ") #-O2")#-std=c11 ")
add_subdirectory(test)
add_subdirectory(src)
set(SOURCE_FILES main.c)

#include_directories(/usr/include/linux/)


add_executable(fsm_main ${SOURCE_FILES} src/fsm.h src/fsm.c src/fsm_queue.h src/fsm_queue.c  src/fsm_debug.h /usr/include/time.h src/fsm_time.h src/fsm_time.c)

add_library(fsm SHARED src/fsm.c src/fsm_queue.c src/fsm_time.c)

install(TARGETS fsm DESTINATION lib)
install(FILES src/fsm.h DESTINATION ../include/cfsm)
