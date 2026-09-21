#!/bin/sh

# Exit if any of the command error & print all commands before execution
set -ex

if [ -d build ]; then
    rm -rf build
fi

mkdir -p build

cd build

cmake -DCMAKE_BUILD_TYPE=Debug ..
