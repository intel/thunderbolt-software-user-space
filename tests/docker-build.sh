#!/bin/bash

# Exit on error
set -e
# Print executed commands
set -x

echo "Building in " `pwd`

rm -rf build && mkdir build
cd build && cmake .. && cmake --build .

LC_ALL=C.UTF-8 make check
