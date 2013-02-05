#!/bin/bash

# WARNING: This is not a makefile! It is provided only to give a sense of the commands
# necessary to build this whole project. 

set -e

echo "Cleaning all make/cmake files"
find -iname '*cmake*' -not -name CMakeLists.txt -not -path 'cmake_modules' -exec rm -rf {} \+
find ./ -type f -name '*.o' -delete
find . -name \*.sw* -type f -delete
rm ./Makefile
rm *.a
rm NEAT/Makefile

set +e
