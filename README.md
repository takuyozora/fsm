# FSM
C API for dynamic state machine.  
This library allow you to create state machines easly, quickly and in an efficient way.

You can :
 - Pass arguments to events to do great things.
 - Create steps with a generic function and some custom arguments.
 - Create instant transitions to weakly seperate two steps.
 - Run multiple state machines because they all are in a separated thread.

## Concepts

- A **step** is an object that define a function to call  and **transitions** to others steps.
- A **transition** represent a link between two **steps**.
- An **event** represent something new and can trigger  **transition** from a **step** to another.
- A **pointer** represent the current **step** of the state machine.

## Quick exemple

```C
#include "fsm.h"

int main(){
    // Creating pointer
    struct fsm_pointer *fsm = create_pointer();
    // Creating steps
    struct fsm_step *step_0 = create_step(fsm_null_callback, NULL);
    struct fsm_step *step_1 = create_step(fsm_null_callback, NULL);
    // Connecting steps together
    connect_step(step_0, step_1, "GO");
    
    // Starting the fsm
    start_pointer(fsm, step_0);
    
    // Signal a new event
    signal_fsm_pointer_of_event(fsm, generate_event("GO", NULL));
    
    join_pointer(fsm);
    destroy_pointer(fsm);
    destroy_all_steps();
    
    return fsm->current_step == step_1;
}
```

## Documentation

You can find the documentation [here](http://takuyozora.github.io/fsm/fsm_8h.html)  

Do not hesitate to read, run and explore the `test/test_fsm.c` file which should be really useful if you want to understand quickly the API.

## Requirement 

For the library :
  - **pthread** for thread mechanisms
  - C standard library
  
For the unittest :
  - **cmocka** for unittest
  - **valgrind** for memory leaks checking
  - *(optional)* **heaptrack** for memory usage tracking
  
For the build process :
  - **cmake**
  - **make**
  - **gcc**
  
For the documentation generation :
  - **doxygen**

## Unittest, doc and build procedure

#### In order to build the entire project
```
git clone https://github.com/takuyozora/fsm.git
cd fsm
mkdir build && cd build
cmake .. && make
```

#### Test it !
```
ctest -V
```

#### RTFM
To generate documentation :
```
cd fsm/doc
./generate_doc.sh
```

You can access it in `fsm/doc/html/index.html`

## Benchmark

*Note that theses benchmark don't test all aspects of the library*

##### Intel(R) Core(TM) i7-4700MQ CPU @ 2.40GHz -- DDR3 Memory at 1600MHz
```
2: [INFO] (test_fsm.c:195) Benchmark for 100000 step transition : 0.005185 s
2: [INFO] (test_fsm.c:196) Benchmark for 1 step transition : ~51.850000 ns
2: [       OK ] benchmark_fsm_direct_transitions
2: [ RUN      ] benchmark_fsm_ping_pong_transitions
2: [INFO] (test_fsm.c:220) Benchmark for 100000 step transition : 0.116433 s
2: [INFO] (test_fsm.c:221) Benchmark for 1 step transition : ~1164.330000 ns
```

##### RaspberyPi B2
```
2: [INFO] (test_fsm.c:195) Benchmark for 100000 step transition : 0.069285 s
2: [INFO] (test_fsm.c:196) Benchmark for 1 step transition : ~692.850000 ns
2: [       OK ] benchmark_fsm_direct_transitions
2: [ RUN      ] benchmark_fsm_ping_pong_transitions
2: [INFO] (test_fsm.c:220) Benchmark for 100000 step transition : 1.027751 s
2: [INFO] (test_fsm.c:221) Benchmark for 1 step transition : ~10277.510000 ns
```


##Licence

Code under GNU GPL v2 Licence.  
If you want a different licence you can contact me to explain your reasons.
