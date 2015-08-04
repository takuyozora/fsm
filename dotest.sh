#!/bin/bash


cd build/
cmake .. && make && ctest -V

exit $?