#!/bin/bash


cd ../build/
cmake .. && make && ctest -V
#heaptrack ./test_fsm -M --massif-threshold --massif-detailed-freq

exit $?