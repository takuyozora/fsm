#!/bin/bash

rm  ../test/snippet/*
for file in ../test/*
do
  if [ -f $file ]; then
    ./conv_file_to_sppinet.sh ../test/$file
  fi
done
doxygen doxygen.conf > /dev/null

exit $?