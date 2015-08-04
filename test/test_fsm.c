//
// Created by olivier on 04/08/15.
//

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "fsm.h"
#include "pthread.h"

void * callback_set_int_to_42(const struct fsm_context *context){
    *(int *)context->fnct_args = 42;
    return NULL;
}

void test_fsm_one(){
    struct fsm_pointer *fsm = create_pointer();
    int value = 5;
    struct fsm_step *step0 = create_step(&callback_set_int_to_42, (void *)&value);
    start_pointer(fsm, step0);
    usleep(1000);
    join_pointer(fsm);
    destroy_pointer(fsm);
    destroy_all_steps();
    assert_int_equal(value, 42);
}


int main(void)
{
    srand ((unsigned int) time(NULL));
    const UnitTest tests[] = {
            unit_test(test_fsm_one),
    };

    int rc = run_tests(tests);
    return rc;
}