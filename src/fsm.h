//
// Created by olivier on 03/08/15.
//

#ifndef FSM_NEW_FSM_H
#define FSM_NEW_FSM_H

/*!
 * \file fsm.h
 * \brief API main header of the C FSM API
 * \author Olivier Radisson <olivier.radisson _at_ insa-lyon.fr>
 * \version 0.1
 *
 * Exemple :
 * @code{.c}
 * // Passing a value to a step in order to modify it
 *
 * #include "fsm.h"
 *
 * void *callback_set_int_from_step_to_42(struct fsm_context *context){
 *   *(int *)context->pointer->current_step->args = 42;
 *   return NULL;
 * }
 *
 * int main(){
 *   struct fsm_pointer *fsm = fsm_create_pointer();
 *   int value = 5;
 *   struct fsm_step *step_0 = fsm_create_step(callback_set_int_from_step_to_42, (void *) &value);
 *   fsm_start_pointer(fsm, step_0);
 *   fsm_join_pointer(fsm);
 *   fsm_delete_pointer(fsm);
 *   fsm_delete_all_steps();
 *
 *   return value == 42;
 * }
 * @endcode
 * */


#include <time.h>
#include <stdbool.h>
#include <bits/time.h>

#include "pthread.h"
#include "fsm_time.h"
#include "fsm_queue.h"


#define MAX_EVENT_UID_LEN 65
#define _EVENT_STOP_POINTER_UID "__STOP_POINTER"
#define _EVENT_DIRECT_TRANSITION_UID "__DIRECT"
#define _EVENT_START_POINTER_UID "__START_POINTER"
#define _EVENT_OUT_ACTION_UID "__OUT_ACTION"
#define _EVENT_TIMEOUT_UID "__TIMEOUT"

#define FSM_STATE_STOPPED  0
#define FSM_STATE_RUNNING  1
#define FSM_STATE_STARTING 2
#define FSM_STATE_CLOSING  3

#define FSM_ERR_NOT_STOPPED 1



struct fsm_event
{
    char uid[MAX_EVENT_UID_LEN];
    struct timespec ttl;
    void * args;
};

struct fsm_context{
    struct fsm_event * event;
    struct fsm_pointer *pointer;
    void* fnct_arg;
};

struct fsm_transition {
    char event_uid[MAX_EVENT_UID_LEN];
    struct fsm_step *next_step;
};

struct fsm_conditional_transition {
    char event_uid[MAX_EVENT_UID_LEN];
    struct fsm_conditional_move (*fnct)(struct fsm_context *);
};

struct fsm_conditional_move {
    bool step;
    void *move;
};

struct fsm_step{
    void * (*fnct)(struct fsm_context *);
    void * args;
    struct fsm_queue * transitions;
    struct fsm_queue * conditional_transitions;
    void * (*out_fnct)(struct fsm_context *);
    void * out_args;
    struct timespec timeout;
    int timeout_us;
};

struct fsm_config_pointer {
    bool ttl_activated;
};

struct fsm_pointer{
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond_event;
    struct fsm_config_pointer config;
    struct fsm_queue input_event;
    struct fsm_queue * ttl_event;
    struct fsm_step * current_step;
    unsigned short running;
};

typedef struct fsm_pointer fsm_pointer;
typedef struct fsm_step fsm_step;
typedef struct fsm_event fsm_event;
typedef struct fsm_transition fsm_transition;
typedef struct fsm_context fsm_context;


/*! Create a pointer. Don't start it, just init variables
 *
 *  @return Pointer to the new created fsm_pointer
 *
 *  Example :
 *  @snippet test_fsm.c test_fsm_start_stop
 *
 *  @note It uses \c malloc for the fsm_pointer allocation : you should free it at the end of his usage
 *  @note The fsm_delete_pointer(fsm_pointer*) function help you to free the pointer correctly
 *
 *  @see fsm_create_pointer_config(struct fsm_config_pointer);
 *
 */
struct fsm_pointer *fsm_create_pointer();

/*! Create a pointer with a custom configuration. Don't start it, just init variables
 *      @param config fsm_config_pointer which hold custom configuration wanted for this fsm_pointer
 *
 *  @return Pointer to the new created fsm_pointer
 *
 *  @note It uses \c malloc for the fsm_pointer allocation : you should free it at the end of his usage
 *  @note The fsm_delete_pointer(fsm_pointer*) function help you to free the pointer correctly
 *
 *  @see fsm_create_pointer();
 *
 *  */
struct fsm_pointer *fsm_create_pointer_config(struct fsm_config_pointer config);

/*! Start the given pointer at the given step in a separated thread
 *      @param _pointer Pointer to the fsm_pointer to start
 *      @param init_step Pointer to the fsm_step which will be reached by the fsm_pointer
 *
 *  @retval 0 if the fsm_pointer correctly started
 *  @retval FSM_ERR_NOT_STOPPED if the fsm_pointer wasn't already stopped
 *
 * Check if the pointer is stopped, and start it in a new thread at the given step.
 *
 * Example:
 * @code{.c}
 * fsm_pointer *fsm = fsm_create_pointer();
 * fsm_step *step_0 = fsm_create_step(fsm_null_callback, NULL);
 *
 * if(fsm_start_pointer(fsm, step_0) != 0){
 *      // Do something about this error
 * }
 *
 * fsm_join_pointer(fsm);
 * fsm_delete_pointer(fsm);
 * fsm_delete_all_steps();
 * @endcode
 *
 * @note This function assure that the first step his launched but not end
 */
unsigned short fsm_start_pointer(struct fsm_pointer *_pointer, struct fsm_step *init_step);

/*! Ask a fsm_pointer to stop and wait for it to join
 *      @param pointer Pointer to the fsm_pointer to join
 */
void fsm_join_pointer(struct fsm_pointer *pointer);

/*! Delete in the straight way the given fsm_pointer
 *      @param pointer Pointer to the fsm_pointer to delete
 *
 * @warning SEGFAULT if the pointer have been already freed
 */
void fsm_delete_pointer(struct fsm_pointer *pointer);

/*! Create a fsm_step pointer based on a callback function and generic void pointer as argument.
 *      @param  fnct Callback function which be called when entering step.
 *      @param  args Pointer which be passed to the callback function. Can be set to \a NULL.
 *
 *  @return Pointer to the new created fsm_step.
 *
 *  @note It uses c malloc for the fsm_step allocation : you should free it at the end of his usage
 *  @note The fsm_delete_all_steps() function allow you the free all allocated steps
 */
struct fsm_step *fsm_create_step(void *(*fnct)(struct fsm_context *), void *args);

/*! Connect two step with an event by creating a transition.
 *      @param from Transition start point.
 *      @param to Transition end point.
 *      @param event_uid UID of the event linking start point to end point.
 *
 * Create a transition for the given event_uid referring to the \a to step pointer and
 * attach it to the \a from step pointer.
 *
 * Example :
 * @code{.c}
 * fsm_step *step_from = fsm_create_step(fsm_null_callback, NULL);
 * fsm_step *step_to = fsm_create_step(fsm_null_callback, NULL);
 *
 * fsm_connect_step(step_from, step_to, "GO");
 *
 * fsm_delete_all_steps();
 * @endcode
 *
 * @note The transition his freed with the step with fsm_delete_all_steps()
 */
void fsm_connect_step(struct fsm_step *from, struct fsm_step *to, char *event_uid);

/*! Delete an unique step
 *      @param step Pointer to the fsm_step to delete
 *
 *  @note Safe if the step have been already freed by fsm_delete_all_steps()
 *  */
void fsm_delete_a_step(fsm_step *step);

/*! Delete all steps which have been created
 *
 * @note Safe if a step have been individual freed by fsm_delete_a_step(fsm_step*)
 */
void fsm_delete_all_steps();

/*! Generate a fsm_event with the given UID and an optional generic void pointer as argument
 *      @param event_uid Event UID string
 *      @param args Generic void pointer to an argument, can be \a NULL
 *
 *  @return Pointer to the new generated fsm_event
 *
 *  @note It uses \c malloc for the fsm_event allocation : you should free it at the end of his usage
 *  @note The event is freed automatically by the fsm process if you directly put it in the input_event fsm_queue from a fsm_pointer with fsm_signal_pointer_of_event(fsm_pointer*,fsm_event*)
 *
 * Example:
 * @code{.c}
 * fsm_pointer *fsm = fsm_create_pointer();
 * fsm_step *step_0 = fsm_create_step(fsm_null_callback, NULL);
 * fsm_step *step_1 = fsm_create_step(fsm_null_callback, NULL);
 *
 * fsm_connect_step(step_0, step_1, "GO");
 * fsm_start_pointer(fsm, step_0);
 *
 * fsm_signal_pointer_of_event(fsm, fsm_generate_event("GO", NULL));
 * fsm_wait_step_blocking(fsm, step_1); // Ensure that the fsm have indeed change his step
 *
 * fsm_join_pointer(fsm);
 * fsm_delete_pointer(fsm);
 * fsm_delete_all_steps();
 * @endcode
 *
 */
struct fsm_event *fsm_generate_event(char *event_uid, void *args);

/*! Signal a fsm_pointer of an event
 *      @param pointer Pointer to the fsm_pointer concern by the event
 *      @param event Pointer to the event to signal
 *
 *  The given fsm_event is copied into the memory and store into the input_event fsm_queue of the given fsm_pointer. So you can't modify it after this.
 *
 *  @see fsm_generate_event(char*,void*)
 */
void fsm_signal_pointer_of_event(struct fsm_pointer *pointer, struct fsm_event *event);

/*! Wait the given pointer to reach the given step in the given timeout interval
 *      @param pointer Pointer to the fsm_pointer to wait
 *      @param step Pointer to the fsm_step the \a pointer must reach
 *      @param mstimeout Timeout value in ms
 *
 *  @retval 0 if the given step have been reached by the pointer
 *  @retval ETIMEDOUT if the step is not reached in the given time
 *
 *  @note Direclty return 0 if the fsm_pointer already is in the given fsm_step
 *
 *  @note Using a monotonic clock to avoid problem if something change system time
 *
 *  @see fsm_wait_step_blocking(fsm_pointer*,fsm_step*)
 *  @see fsm_wait_leaving_step_blocking(fsm_pointer*,fsm_step*)
 *  @see fsm_wait_leaving_step_mstimeout(fsm_pointer*,fsm_step*,unsigned int)
 *  */
int fsm_wait_step_mstimeout(struct fsm_pointer *pointer, struct fsm_step *step, unsigned int mstimeout);

/*! Wait the given pointer to reach the given step.
 *      @param pointer Pointer to the fsm_pointer to wait
 *      @param step Pointer to the fsm_step the \a pointer must reach
 *
 *  @note Direclty return if the fsm_pointer already is in the given fsm_step
 *
 *  @see fsm_wait_step_mstimeout(fsm_pointer*,fsm_step*,unsigned int)
 *  @see fsm_wait_leaving_step_blocking(fsm_pointer*,fsm_step*)
 *  @see fsm_wait_leaving_step_mstimeout(fsm_pointer*,fsm_step*,unsigned int)
 *  */
void fsm_wait_step_blocking(struct fsm_pointer *pointer, struct fsm_step *step);

/*! Wait the given pointer to leave the given step.
 *      @param pointer Pointer to the fsm_pointer to wait
 *      @param step Pointer to the fsm_step the \a pointer must leave
 *
 *  @note Direclty return if the fsm_pointer already isn't in the given fsm_step
 *
 *  @see fsm_wait_step_mstimeout(fsm_pointer*,fsm_step*,unsigned int)
 *  @see fsm_wait_step_blocking(fsm_pointer*,fsm_step*)
 *  @see fsm_wait_leaving_step_mstimeout(fsm_pointer*,fsm_step*,unsigned int)
 *  */
void fsm_wait_leaving_step_blocking(struct fsm_pointer *pointer, struct fsm_step *step);

/*! Wait the given pointer to leave the given step in the given timeout interval
 *      @param pointer Pointer to the fsm_pointer to wait
 *      @param step Pointer to the fsm_step the \a pointer must leave
 *      @param mstimeout Timeout value in ms
 *
 *  @retval 0 if the given step have been leaved by the pointer
 *  @retval ETIMEDOUT if the step is not leaved in the given time
 *
 *  @note Direclty return 0 if the fsm_pointer already isn't in the given fsm_step
 *
 *  @note Using a monotonic clock to avoid problem if something change system time
 *
 *  @see fsm_wait_step_mstimeout(fsm_pointer*,fsm_step*,unsigned int)
 *  @see fsm_wait_step_blocking(fsm_pointer*,fsm_step*)
 *  @see fsm_wait_leaving_step_blocking(fsm_pointer*,fsm_step*)
 *  */
int fsm_wait_leaving_step_mstimeout(struct fsm_pointer *pointer, struct fsm_step *step, unsigned int mstimeout);

/*! Set a timeout to the given step in microseconds
 *      @param step Pointer to the step
 *      @param timeout_us Timeout in microseconds
 *
 *  If there is no transition after the given timeout an _EVENT_TIMEOUT_UID will be put in the event queue
 */
void fsm_set_timeout_to_step(struct fsm_step *step, int timeout_us);

/*! Add a conditional transition to the given step
 *      @param step Pointer to the step
 *      @param event_uid UID of the event which triggered the conditional transition
 *      @param fnct Function of the conditional transition, return the next_step or NULL
 *
 *  If the given event_uid appears, the fnct is called with the actual context. If the function return NULL, the fsm do not change his current step. Otherwise, the fsm jump to the step returned by the function.
 */
void fsm_add_conditional_transition_to_step(struct fsm_step *step, char event_uid[MAX_EVENT_UID_LEN], struct fsm_conditional_move (*fnct)(struct fsm_context *));

struct fsm_conditional_move fsm_cond_return_step(fsm_step * step);

struct fsm_conditional_move fsm_cond_return_conditional_transition(struct fsm_conditional_move (*fnct)(struct fsm_context *));

/*! Useless function which return a NULL pointer to create wait steps
 *      @param context Pointer to the fsm_context in which one the function is called.
 *
 *  @retval NULL
 *  */
void * fsm_null_callback(struct fsm_context *context);



#endif //FSM_NEW_FSM_H
