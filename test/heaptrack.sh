#!/bin/bash

cd /tmp

rm heaptrack.test* massif.out*
heaptrack "$1" -M --massif-threshold --massif-detailed-freq > /dev/null &&
heaptrack_print -M massif.out heaptrack.test* > /dev/null&&
massif-visualizer massif.out* 2&> /dev/null