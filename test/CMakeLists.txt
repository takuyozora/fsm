include_directories ("${PROJECT_SOURCE_DIR}/src")
include_directories ("${PROJECT_SOURCE_DIR}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage -fPIC -O0 -pg")

add_executable(test_queue test_queue.c
${PROJECT_SOURCE_DIR}/src/fsm_queue.c
${PROJECT_SOURCE_DIR}/src/fsm_queue.h benchmark.h)
#SET_SOURCE_FILES_PROPERTIES( ../src/fsm_time.c PROPERTIES CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wno-unused-variable  -lpthread -D_REENTRANT -std=gnu11 -Wl,--wrap=clock_gettime")
#set_source_files_properties(../src/fsm_time.c PROPERTIES COMPILE_FLAGS -Wl)
#set_source_files_properties(../src/fsm_time.c PROPERTIES COMPILE_FLAGS -Wl,-wrap,clock_gettime)
#set_source_files_properties(test_time.c PROPERTIES COMPILE_FLAGS -Wl,-wrap,clock_gettime)
#add_executable(test_time test_time.c ../src/fsm_time.c ../src/fsm_time.h)

add_library(TIME_LIB ../src/fsm_time.c ../src/fsm_time.h wrapper.h ../src/fsm_debug.h)
set_target_properties(TIME_LIB PROPERTIES COMPILE_FLAGS "-D DBG_EXE -D DBG_TEST_EXE -D DBG_VERBOSE")
SET(TIME_LIB_LINK_LIBRARIES "-Wl,-wrap,clock_gettime" TIME_LIB)
add_executable(test_time test_time.c  wrapper.h ../src/fsm_debug.h wrapper.c wrapper.h)
#set_target_properties(test_time PROPERTIES COMPILE_FLAGS "-D DBG_EXE -D DBG_TEST_EXE -D DBG_VERBOSE")

TARGET_LINK_LIBRARIES(test_time ${TIME_LIB_LINK_LIBRARIES})

#set_target_properties(test_time PROPERTIES CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wl,--wrap=clock_gettime")
add_executable(test_fsm test_fsm.c
${PROJECT_SOURCE_DIR}/src/fsm.h
${PROJECT_SOURCE_DIR}/src/fsm.c
${PROJECT_SOURCE_DIR}/src/fsm_queue.c
${PROJECT_SOURCE_DIR}/src/fsm_queue.h ../src/fsm_time.h ../src/fsm_time.c)
#add_executable(test+_fsm test+_fsm.c
#${PROJECT_SOURCE_DIR}/src/fsm.h
#${PROJECT_SOURCE_DIR}/src/fsm.c
#${PROJECT_SOURCE_DIR}/src/fsm_queue.c
#${PROJECT_SOURCE_DIR}/src/fsm_queue.h benchmark.h ../src/fsm_time.h ../src/fsm_time.c)
#add_dependencies(test_queue fsm_queue)
#add_dependencies(test_fsm fsm_queue fsm)
add_test(test_queue test_queue)
add_test(test_queue_valgrind valgrind
            --error-exitcode=1 --read-var-info=yes
            --leak-check=full --show-leak-kinds=all
            --track-origins=yes
            ./test_queue)

add_test(test_time test_time)
add_test(test_time_valgrind valgrind
            --error-exitcode=1 --read-var-info=yes
            --leak-check=full --show-leak-kinds=all
            --track-origins=yes
            ./test_time)

add_test(test_fsm test_fsm)
#add_test(test+_fsm test+_fsm)
add_test(test_fsm_valgrind valgrind
            --error-exitcode=1 --read-var-info=yes
            --leak-check=full --show-leak-kinds=all
            --track-origins=yes
            ./test_fsm)

#add_test(test_fsm_heaptrack ${PROJECT_SOURCE_DIR}/test/heaptrack.sh ${PROJECT_SOURCE_DIR}/build/test/test_fsm)
#add_test(test_fsm+_heaptrack ${PROJECT_SOURCE_DIR}/test/heaptrack.sh ${PROJECT_SOURCE_DIR}/build/test/test+_fsm)

target_link_libraries(test_queue cmocka)
target_link_libraries(test_time cmocka)
target_link_libraries(test_fsm cmocka)
#target_link_libraries(test+_fsm cmocka)