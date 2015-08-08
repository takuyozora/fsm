#!/bin/bash

rm  ../test/snippet/*
for file in ../test/*
do
  if [ -f $file ]; then
    ./generate_snippet.sh ../test/$file
  fi
done
doxygen doxygen.conf > /dev/null

exit $?