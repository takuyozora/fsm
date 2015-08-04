include_directories ("${PROJECT_SOURCE_DIR}/src")
include_directories ("${PROJECT_SOURCE_DIR}")

add_executable(test_queue test_queue.c
${PROJECT_SOURCE_DIR}/src/fsm_queue.c
${PROJECT_SOURCE_DIR}/src/fsm_queue.h
${PROJECT_SOURCE_DIR}/src/fsm_event_queue.c
${PROJECT_SOURCE_DIR}/src/fsm_event_queue.h
${PROJECT_SOURCE_DIR}/src/fsm_transition_queue.c
${PROJECT_SOURCE_DIR}/src/fsm_transition_queue.h)
add_executable(test_fsm test_fsm.c
${PROJECT_SOURCE_DIR}/src/fsm.h
${PROJECT_SOURCE_DIR}/src/fsm.c
${PROJECT_SOURCE_DIR}/src/fsm_queue.c
${PROJECT_SOURCE_DIR}/src/fsm_queue.h
${PROJECT_SOURCE_DIR}/src/fsm_event_queue.c
${PROJECT_SOURCE_DIR}/src/fsm_event_queue.h
${PROJECT_SOURCE_DIR}/src/fsm_transition_queue.c
${PROJECT_SOURCE_DIR}/src/fsm_transition_queue.h)
#add_dependencies(test_queue fsm_queue)
#add_dependencies(test_fsm fsm_queue fsm)
add_test(test_queue test_queue)
add_test(test_fsm test_fsm)
add_test(test_fsm_valgrind valgrind
         --error-exitcode=1 --read-var-info=yes
         --leak-check=full --show-leak-kinds=all
         --track-origins=yes
         ./test_fsm)
add_test(test_queue_valgrind valgrind
                  --error-exitcode=1 --read-var-info=yes
                  --leak-check=full --show-leak-kinds=all
                  --track-origins=yes
                  ./test_queue)
target_link_libraries(test_queue cmocka)
target_link_libraries(test_fsm cmocka)