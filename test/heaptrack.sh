#!/bin/bash

cd /tmp

rm heaptrack.test_fsm.* massif.out*
heaptrack "$1" -M --massif-threshold --massif-detailed-freq > /dev/null &&
heaptrack_print -M massif.out heaptrack.test_fsm.* > /dev/null&&
massif-visualizer massif.out* 2&> /dev/null