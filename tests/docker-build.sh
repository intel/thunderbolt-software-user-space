#!/bin/bash

# Exit on error
set -e
# Print executed commands
set -x

# Building in /usr/local/src
pushd ..
# Required commits are: bd14fb6 and 88d2cc3
git clone https://github.com/martinpitt/umockdev.git
pushd umockdev
./autogen.sh --prefix=/usr && make -j4 && make install && popd
popd

echo "Returning to " `pwd`

mkdir -p build && cd build && cmake .. && cmake --build .
export LC_ALL=C.UTF-8
make check
